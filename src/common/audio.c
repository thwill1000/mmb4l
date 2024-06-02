/*-*****************************************************************************

MMBasic for Linux (MMB4L)

audio.c

Copyright 2021-2024 Geoff Graham, Peter Mather and Thomas Hugo Williams.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holders nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

4. The name MMBasic be used when referring to the interpreter in any
   documentation and promotional material and the original copyright message
   be displayed  on the console at startup (additional copyright messages may
   be added).

5. All advertising materials mentioning features or use of this software must
   display the following acknowledgement: This product includes software
   developed by Geoff Graham, Peter Mather and Thomas Hugo Williams.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

#include "audio.h"
#include "audio_tables.h"
#include "cstring.h"
#include "error.h"
#include "events.h"
#include "file.h"
#include "memory.h"
#include "mmresult.h"
#include "path.h"
#include "utility.h"
#include "../third_party/hxcmod.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#define BUFFER_SIZE        1
#define AUDIO_SAMPLE_RATE  44100UL
#define PWM_FREQ           AUDIO_SAMPLE_RATE
#define WAV_BUFFER_SIZE    16384

typedef enum {
    P_NOTHING,
    P_PAUSE_TONE,
    P_TONE,
    P_PAUSE_SOUND,
    P_SOUND,
    P_WAV,
    P_PAUSE_WAV,
    P_FLAC,
    P_MP3,
    P_MOD,
    P_PAUSE_MOD,
    P_PAUSE_FLAC,
    P_TTS,
    P_DAC,
    P_PAUSE_MP3
} AudioState;

static uint64_t bcount[3] = {0, 0, 0};  // Number of bytes(?) of data in audio buffer.
                                        // I believe element 0 is ignored.
static uint64_t bcounte[3] = {0, 0, 0};
static AudioState audio_state = P_NOTHING;
//static AudioState audio_statee = P_NOTHING;
static float fFilterVolumeL = 1.0f;
static float fFilterVolumeR = 1.0f;
//static int mono;
static int nextbuf = 0;  // Index of the buffer to fill.
static int nextbufe = 0;
static float PhaseM_left = 0.0f;
static float PhaseM_right = 0.0f;
static float PhaseAC_left = 0.0f;
static float PhaseAC_right = 0.0f;
static int playreadcomplete = 1;
static int playreadcompletee = 1;
static uint64_t ppos = 0;  // Playing position in the currently playing buffer.
//static uint64_t ppose = 0;
static uint64_t SoundPlay;
static char *sbuff1 = NULL;
static char *sbuff1e = NULL;
static char *sbuff2 = NULL;
static char *sbuff2e = NULL;
static const unsigned short *sound_mode_left[MAXSOUNDS] = {null_table, null_table, null_table,
                                                           null_table};
static const unsigned short *sound_mode_right[MAXSOUNDS] = {null_table, null_table, null_table,
                                                            null_table};
static float sound_PhaseAC_left[MAXSOUNDS] = {0};
static float sound_PhaseAC_right[MAXSOUNDS] = {0};
static float sound_PhaseM_left[MAXSOUNDS] = {0};
static float sound_PhaseM_right[MAXSOUNDS] = {0};
static int sound_v_left[MAXSOUNDS] = {0};
static int sound_v_right[MAXSOUNDS] = {0};
static int swingbuf = 0;  // Index of the buffer to play.
static int swingbufe = 0;
static int vol_left = 100;
static int vol_right = 100;
// static int wav_filesize;
static int WAVcomplete;

static const char* NO_ERROR = "";
static bool audio_initialised = false;
static char* audio_modbuff = NULL;
static const char* audio_wav_interrupt = NULL;
static int audio_fnbr = 0;
static modcontext audio_mod_context = { 0 };
static bool audio_mod_noloop = false;

#define CloseAudio(x) audio_close(x)

static void audio_callback(void *userdata, Uint8 *stream, int len);

MmResult audio_init() {
    if (audio_initialised) return kOk;
    MmResult result = events_init();
    if (FAILED(result)) return result;

    SDL_AudioSpec spec = {
      .format = AUDIO_F32,
      .channels = 2,
      .freq = AUDIO_SAMPLE_RATE,
      .samples = BUFFER_SIZE,
      .callback = audio_callback,
	};
    if (FAILED(SDL_OpenAudio(&spec, NULL))) error_throw(kAudioApiError);

    SDL_PauseAudio(0);
    audio_tables_init();
    audio_initialised = true;
    return kOk;
}

const char *audio_last_error() {
    const char* emsg = SDL_GetError();
    return emsg && *emsg ? emsg : NO_ERROR;
}

MmResult audio_close(bool all) {
    if (!audio_initialised) return kOk;

    SDL_LockAudioDevice(1);

    // int was_playing = audio_state;
    audio_state = P_NOTHING;
    memset(bcount, 0, sizeof(bcount));
    swingbuf = nextbuf = playreadcomplete = 0;
    memset(bcounte, 0, sizeof(bcounte));
    swingbufe = nextbufe = playreadcompletee = 0;
    // WAVInterrupt = NULL;
    // if (was_playing == P_MP3 || was_playing == P_PAUSE_MP3) drmp3_uninit(&mymp3);
    // if (was_playing == P_FLAC || was_playing == P_PAUSE_FLAC) FreeMemorySafe((void **)&myflac);
    // ForceFileClose(WAV_fnbr);
    // FreeMemorySafe((void **)&modbuff);
    FreeMemory((void *) sbuff1); sbuff1 = NULL; // FreeMemorySafe((void **)&sbuff1);
    FreeMemory((void *) sbuff2); sbuff2 = NULL; // FreeMemorySafe((void **)&sbuff2);
    FreeMemory((void *) sbuff1e); sbuff1e = NULL; // FreeMemorySafe((void **)&sbuff1e);
    FreeMemory((void *) sbuff2e); sbuff2e = NULL; // FreeMemorySafe((void **)&sbuff2e);
    // FreeMemorySafe((void **)&mymp3);
    // memset(&mywav, 0, sizeof(drwav));
    // if (all) {
    //     FreeMemorySafe((void **)&alist);
    //     trackstoplay = 0;
    //     trackplaying = 0;
    // }
    SoundPlay = 0;
    ppos = 0;
    for (int i = 0; i < MAXSOUNDS; i++) {
        sound_PhaseM_left[i] = 0;
        sound_PhaseM_right[i] = 0;
        sound_PhaseAC_left[i] = 0;
        sound_PhaseAC_right[i] = 0;
        sound_mode_left[i] = (uint16_t *)null_table;
        sound_mode_right[i] = (uint16_t *)null_table;
    }

    SDL_UnlockAudioDevice(1);

    return kOk;
}

static float audio_callback_tone(int nChannel) {
    if (!SoundPlay) {
        CloseAudio(1);
        WAVcomplete = true;
        return 0.0f;
    } else {
        int v;
        SoundPlay--;
        if (nChannel == 0) {
            v = ((((sine_table[(int)PhaseAC_left] - 2000) * mapping[vol_left]) / 2000) + 2000);
            PhaseAC_left = PhaseAC_left + PhaseM_left;
            if (PhaseAC_left >= 4096.0) PhaseAC_left -= 4096.0;
        } else {
            v = ((((sine_table[(int)PhaseAC_right] - 2000) * mapping[vol_right]) / 2000) + 2000);
            PhaseAC_right = PhaseAC_right + PhaseM_right;
            if (PhaseAC_right >= 4096.0) PhaseAC_right -= 4096.0;
        }
        return ((float)v - 2000.0f) / 2000.0f;
    }
}

static float audio_callback_mod(int nChannel) {
    float value = 0, valuee = 0;

    if (swingbuf) {  // buffer is primed
        int16_t *buf = (swingbuf == 1)
                ? (int16_t *)sbuff1
                : (int16_t *)sbuff2;
        //printf("ppos = %ld, bcount[swingbuf] = %ld\n", ppos, bcount[swingbuf]);
        if (ppos < bcount[swingbuf]) {
            //printf("flacbuff[%ld] = %d\n", ppos, flacbuff[ppos]);
            value = (float)buf[ppos++] / 32768.0f *
                    (nChannel == 0 ? fFilterVolumeL : fFilterVolumeR);
        }

        // If we are now at the end of the buffer ...
        if (ppos == bcount[swingbuf]) {
            if (bcount[swingbuf == 1 ? 2 : 1] != 0) {
                // Alternative buffer is not empty so swap buffer.
                bcount[swingbuf] = 0;
                swingbuf = swingbuf == 1 ? 2 : 1;
            } else {
                // Alternative buffer is empty so replay current buffer.
                nextbuf = swingbuf == 1 ? 2 : 1;
            }
            ppos = 0;
        }
    }
    // if (audio_statee == P_WAV) {
    //     static int toggle = 0;
    //     float *flacbuff;
    //     if (swingbufe == 1)
    //         flacbuff = (float *)sbuff1e;
    //     else
    //         flacbuff = (float *)sbuff2e;
    //     if (ppose < bcounte[swingbufe]) {
    //         if (mono) {
    //             if (toggle)
    //                 valuee = (float)flacbuff[ppose++] *
    //                          (nChannel == 0 ? fFilterVolumeL : fFilterVolumeR);
    //             else
    //                 valuee =
    //                     (float)flacbuff[ppose] * (nChannel == 0 ? fFilterVolumeL : fFilterVolumeR);
    //             toggle = !toggle;
    //         } else {
    //             valuee =
    //                 (float)flacbuff[ppose++] * (nChannel == 0 ? fFilterVolumeL : fFilterVolumeR);
    //         }
    //     }
    //     if (ppose == bcounte[swingbufe]) {
    //         int psave = ppose;
    //         bcounte[swingbufe] = 0;
    //         ppose = 0;
    //         if (swingbufe == 1)
    //             swingbufe = 2;
    //         else
    //             swingbufe = 1;
    //         if (bcounte[swingbufe] == 0 && !playreadcompletee) {  // nothing ready yet so flip back
    //             if (swingbufe == 1) {
    //                 swingbufe = 2;
    //                 nextbufe = 1;
    //             } else {
    //                 swingbufe = 1;
    //                 nextbufe = 2;
    //             }
    //             bcounte[swingbufe] = psave;
    //             ppose = 0;
    //         }
    //     }
    // }
    //printf("value = %g\n", value + valuee);
    return value + valuee;
}

// Also flac.
static float audio_callback_mp3(int nChannel) {
    float *flacbuff;
    float value = 0;
    ;
    if (bcount[1] == 0 && bcount[2] == 0 && playreadcomplete == 1) {
        //				HAL_TIM_Base_Stop_IT(&htim4);
    }
    if (swingbuf) {  // buffer is primed
        if (swingbuf == 1)
            flacbuff = (float *)sbuff1;
        else
            flacbuff = (float *)sbuff2;
        if (ppos < bcount[swingbuf]) {
            value = (float)flacbuff[ppos++] * (nChannel == 0 ? fFilterVolumeL : fFilterVolumeR);
        }
        if (ppos == bcount[swingbuf]) {
            int psave = ppos;
            bcount[swingbuf] = 0;
            ppos = 0;
            if (swingbuf == 1)
                swingbuf = 2;
            else
                swingbuf = 1;
            if (bcount[swingbuf] == 0 && !playreadcomplete) {  // nothing ready yet so flip back
                if (swingbuf == 1) {
                    swingbuf = 2;
                    nextbuf = 1;
                } else {
                    swingbuf = 1;
                    nextbuf = 2;
                }
                bcount[swingbuf] = psave;
                ppos = 0;
            }
        }
    }
    return value;
}

static float audio_callback_sound(int nChannel) {
    static int noisedwellleft[MAXSOUNDS] = {0}, noisedwellright[MAXSOUNDS] = {0};
    static uint32_t noiseleft[MAXSOUNDS] = {0}, noiseright[MAXSOUNDS] = {0};
    int i, j;
    int leftv = 0, rightv = 0;
    for (i = 0; i < MAXSOUNDS; i++) {  // first update the 8 sound pointers
        if (nChannel == 0) {
            if (sound_mode_left[i] != null_table) {
                // printf("Hello 0\n");
                if (sound_mode_left[i] != white_noise_table) {
                    // printf("Hello 0\n");
                    sound_PhaseAC_left[i] = sound_PhaseAC_left[i] + sound_PhaseM_left[i];
                    if (sound_PhaseAC_left[i] >= 4096.0) sound_PhaseAC_left[i] -= 4096.0;
                    j = (int)sound_mode_left[i][(int)sound_PhaseAC_left[i]];
                    j = (j - 2000) * mapping[sound_v_left[i]] / 2000;
                    leftv += j;
                } else {
                    if (noisedwellleft[i] <= 0) {
                        noisedwellleft[i] = (int)sound_PhaseM_left[i];
                        noiseleft[i] = rand() % 3700 + 100;
                    }
                    if (noisedwellleft[i]) noisedwellleft[i]--;
                    j = (int)noiseleft[i];
                    j = (j - 2000) * mapping[sound_v_left[i]] / 2000;
                    leftv += j;
                }
            }
        } else {
            if (sound_mode_right[i] != null_table) {
                // printf("Hello 1\n");
                if (sound_mode_right[i] != white_noise_table) {
                    // printf("Hello 1\n");
                    sound_PhaseAC_right[i] = sound_PhaseAC_right[i] + sound_PhaseM_right[i];
                    if (sound_PhaseAC_right[i] >= 4096.0) sound_PhaseAC_right[i] -= 4096.0;
                    j = (int)sound_mode_right[i][(int)sound_PhaseAC_right[i]];
                    j = (j - 2000) * mapping[sound_v_right[i]] / 2000;
                    rightv += j;
                } else {
                    if (noisedwellright[i] <= 0) {
                        noisedwellright[i] = (int)sound_PhaseM_right[i];
                        noiseright[i] = rand() % 3700 + 100;
                    }
                    if (noisedwellright[i]) noisedwellright[i]--;
                    j = (int)noiseright[i];
                    j = (j - 2000) * mapping[sound_v_right[i]] / 2000;
                    rightv += j;
                }
            }
        }
    }
    leftv += 2000;
    rightv += 2000;
    // printf("leftv = %d, rightv = %d\n", leftv, rightv);
    if (nChannel == 0)
        return ((float)leftv - 2000.0f) / 2000.0f;
    else
        return ((float)rightv - 2000.0f) / 2000.0f;
}

static void audio_callback(void *userdata, Uint8 *stream, int len) {
    // static int counter = 0;
    float *fstream = (float *)stream;
    for (int i = 0; i < BUFFER_SIZE; i += 2) {
        switch (audio_state) {
            case P_TONE:
                fstream[i] = audio_callback_tone(0);
                fstream[i + 1] = audio_callback_tone(1);
                break;
            case P_MOD:
                fstream[i] = audio_callback_mod(0);
                fstream[i + 1] = audio_callback_mod(1);
                break;
            case P_MP3:
            case P_WAV:
            case P_FLAC:
                fstream[i] = audio_callback_mp3(0);
                fstream[i + 1] = audio_callback_mp3(1);
                break;
            case P_SOUND:
                fstream[i] = audio_callback_sound(0);
                fstream[i + 1] = audio_callback_sound(1);
                break;
            default:
                fstream[i] = 0.0f;
                fstream[i + 1] = 0.0f;
                break;
        }
        // printf("%d: %g, %g\n", i, fstream[i], fstream[i + 1]);
        // counter++;
        // if (counter == 1000) exit(0);
    }
}

MmResult audio_pause() {
    if (!audio_initialised) {
        MmResult result = audio_init();
        if (FAILED(result)) return result;
    }

    SDL_LockAudioDevice(1);

    MmResult result = kOk;
    switch (audio_state) {
        case P_TONE:
            audio_state = P_PAUSE_TONE;
            break;
        case P_FLAC:
            audio_state = P_PAUSE_FLAC;
            break;
        case P_SOUND:
            audio_state = P_PAUSE_SOUND;
            break;
        case P_MOD:
            audio_state = P_PAUSE_MOD;
            break;
        case P_WAV:
            audio_state = P_PAUSE_WAV;
            break;
        case P_MP3:
            audio_state = P_PAUSE_MP3;
            break;
        default:
            result = kAudioNothingToPause;
            break;
    }

    SDL_UnlockAudioDevice(1);
    return result;
}

MmResult audio_play_modfile(const char *filename, const char *interrupt) {
    if (!audio_initialised) {
        MmResult result = audio_init();
        if (FAILED(result)) return result;
    }

    SDL_LockAudioDevice(1);

    if (audio_state != P_NOTHING) {
        SDL_UnlockAudioDevice(1);
        return kSoundInUse;
    }

    uint32_t sample_rate = 44100; // 16000; // 44100
    sbuff1 = (char *)GetMemory(WAV_BUFFER_SIZE);
    sbuff2 = (char *)GetMemory(WAV_BUFFER_SIZE);

    char filename2[STRINGSIZE];
    cstring_cpy(filename2, filename, STRINGSIZE);
    if (!path_has_suffix(filename2, ".MOD", true)) {
        // TODO: What if the file-extension is ".mod" ?
        MmResult result = cstring_cat(filename2, ".MOD", STRINGSIZE);
        if (FAILED(result)) return result;
    }

    audio_wav_interrupt = interrupt;
    audio_mod_noloop = interrupt != NULL;
    WAVcomplete = 0;

    // Read the file.
    // TODO: Could leave audio device locked!
    audio_fnbr = file_find_free();
    MmResult result = file_open(filename2, "rb", audio_fnbr);
    if (FAILED(result)) return result;
    int size = file_lof(audio_fnbr);
    audio_modbuff = (char *)GetMemory(size + 256);
    file_read(audio_fnbr, audio_modbuff, size);
    result = file_close(audio_fnbr);
    if (FAILED(result)) return result;

    // for (int i = 0; i < size; ++i) {
    //     printf("%x, ", audio_modbuff[i]);
    // }
    // printf("\n");

    // char filename[STRINGSIZE] = { 0 };
    // char* p, * r;
    // int i = 0, size;
    // modfilesamplerate = 44100;
    // p = (char *)getFstring(argv[0]);                                    // get the file name
	// fullfilename(p, filename, ".MOD");
	// WAVInterrupt = NULL;
    // WAVcomplete = 0;
    // // open the file
	// if (argc == 3)modfilesamplerate = (int)getinteger(argv[2]);
	// if (!(modfilesamplerate == 8000 || modfilesamplerate == 16000 || modfilesamplerate == 22050 || modfilesamplerate == 44100 || modfilesamplerate == 48000))error((char *)"Valid rates are 8000, 16000, 22050, 44100, 48000");
    // WAV_fnbr = FindFreeFileNbr();
    // if (!BasicFileOpen(filename, WAV_fnbr, (char *)"rb")) return;
    // i = 0;
    // fseek(FileTable[WAV_fnbr].fptr, 0L, SEEK_END);
    // size = ftell(FileTable[WAV_fnbr].fptr);
    // fseek(FileTable[WAV_fnbr].fptr, 0L, SEEK_SET);
    // modbuff = r = (char *)GetMemory(size + 256);
    // while (!MMfeof(WAV_fnbr)) {                                     // while waiting for the end of file
    //     *r++ = FileGetChar(WAV_fnbr);
    //     i++;
    // }
    // FileClose(WAV_fnbr);
    hxcmod_init(&audio_mod_context);
    hxcmod_setcfg(&audio_mod_context, sample_rate, 1, 1);
    hxcmod_load(&audio_mod_context, (void*)audio_modbuff, size);
    hxcmod_fillbuffer(&audio_mod_context, (msample*)sbuff1, WAV_BUFFER_SIZE / 4, NULL,
                      audio_mod_noloop ? 1 : 0);
    // for (int i = 0; i < WAV_BUFFER_SIZE / 4; ++i) {
    //     printf("%x, ", sbuff1[i]);
    // }
    // // }
    // printf("\n");
    // exit(0);

 //       wav_filesize = WAV_BUFFER_SIZE / 2;
    bcount[1] = WAV_BUFFER_SIZE / 2;
    bcount[2] = 0;
	// (SampleRate != modfilesamplerate || NChannels != 2) {
	// SampleRate = modfilesamplerate;
	// NChannels = 2;
	// SystemMode = MODE_SAMPLERATE;
	// while (SystemMode != MODE_RUN) {}
	// }
    swingbuf = 1;
    nextbuf = 2;
    ppos = 0;
    playreadcomplete = 0;
    audio_state = P_MOD;

    SDL_UnlockAudioDevice(1);
    return kOk;
}

MmResult audio_play_modsample(uint8_t sample_num, uint8_t channel_num, uint8_t volume,
                              uint32_t sample_rate) {
    if (!audio_initialised) {
        MmResult result = audio_init();
        if (FAILED(result)) return result;
    }

    SDL_LockAudioDevice(1);

    if (audio_state != P_MOD) {
        SDL_UnlockAudioDevice(1);
        return kAudioNoModFile;
    }

    hxcmod_playsoundeffect(&audio_mod_context, sample_num - 1, channel_num - 1, max(0, volume - 1),
            3579545 / sample_rate);

    SDL_UnlockAudioDevice(1);
    return kOk;
}

MmResult audio_play_sound(uint8_t sound_no, Channel channel, SoundType type, float frequency,
                          uint8_t volume) {
    if (!audio_initialised) {
        MmResult result = audio_init();
        if (FAILED(result)) return result;
    }

    SDL_LockAudioDevice(1);

    if (!(audio_state == P_NOTHING || audio_state == P_SOUND ||
          audio_state == P_PAUSE_SOUND)) {
        SDL_UnlockAudioDevice(1);
        return kSoundInUse;
    }

    // sound_no--; // In BASIC this is 1-4, but in C it is 0-3.
    // float f_in, PhaseM;
	// int channel, left = 0, right = 0;
	// uint16_t* lastleft = NULL, * lastright = NULL;
	// setnoise(); 
	// WAV_fnbr = 0;
	// channel = (int)getint(argv[0], 1, MAXSOUNDS) - 1;
	uint16_t *lastleft = (uint16_t*)sound_mode_left[sound_no];
	uint16_t *lastright = (uint16_t*)sound_mode_right[sound_no];

    // printf("Type = %d\n", type);
    // printf("Sound no = %d\n", sound_no);
    // printf("sine_table = %p\n", sine_table);

    switch (type) {
        case kSoundTypeSine:
            if (channel & kChannelLeft) sound_mode_left[sound_no] = (uint16_t *) sine_table;
            if (channel & kChannelRight) sound_mode_right[sound_no] = (uint16_t *) sine_table;
            break;
        case kSoundTypeSquare:
            if (channel & kChannelLeft) sound_mode_left[sound_no] = (uint16_t *) square_table;
            if (channel & kChannelRight) sound_mode_right[sound_no] = (uint16_t *) square_table;
            break;
        case kSoundTypeTriangular:
            if (channel & kChannelLeft) sound_mode_left[sound_no] = (uint16_t *) triangular_table;
            if (channel & kChannelRight) sound_mode_right[sound_no] = (uint16_t *) triangular_table;
            break;
        case kSoundTypeSawTooth:
            if (channel & kChannelLeft) sound_mode_left[sound_no] = (uint16_t *) saw_tooth_table;
            if (channel & kChannelRight) sound_mode_right[sound_no] = (uint16_t *) saw_tooth_table;
            break;
        case kSoundTypePeriodicNoise:
            if (channel & kChannelLeft) sound_mode_left[sound_no] = (uint16_t *) periodic_noise_table;
            if (channel & kChannelRight) sound_mode_right[sound_no] = (uint16_t *) periodic_noise_table;
            break;
        case kSoundTypeWhiteNoise:
            if (channel & kChannelLeft) sound_mode_left[sound_no] = (uint16_t *) white_noise_table;
            if (channel & kChannelRight) sound_mode_right[sound_no] = (uint16_t *) white_noise_table;
            break;
        case kSoundTypeNull:
            if (channel & kChannelLeft) sound_mode_left[sound_no] = (uint16_t *) null_table;
            if (channel & kChannelRight) sound_mode_right[sound_no] = (uint16_t *) null_table;
            break;
        default:
            SDL_UnlockAudioDevice(1);
            return kInternalFault;
    }

    // printf("sound_mode_left = %p\n", sound_mode_left[sound_no]);
    // printf("sound_mode_right = %p\n", sound_mode_right[sound_no]);

    // Should it be < 1.0 ? "Valid is 1Hz to 20KHz"
    if (frequency < 0.0 || frequency > 20000.0) {
        SDL_UnlockAudioDevice(1);
        return kSoundInvalidFrequency;
    }

	// f_in = 10.0;

	if (channel & kChannelLeft) {
        const float phase_m = sound_mode_left[sound_no] == white_noise_table
                ? frequency
                : frequency / (float)PWM_FREQ * 4096.0f;
		if (lastleft == (uint16_t*) null_table) sound_PhaseAC_left[sound_no] = 0.0;
		sound_PhaseM_left[sound_no] = phase_m;
        sound_v_left[sound_no] = (volume * 41) / 25;
        // printf("LEFT: %g, %d\n", phase_m, sound_v_left[sound_no]);
	}

	if (channel & kChannelRight) {
        const float phase_m = sound_mode_right[sound_no] == white_noise_table
                ? frequency
                : frequency / (float)PWM_FREQ * 4096.0f;
		if (lastright == (uint16_t*) null_table) sound_PhaseAC_right[sound_no] = 0.0;
		sound_PhaseM_right[sound_no] = phase_m;
        sound_v_right[sound_no] = (volume * 41) / 25;
        // printf("RIGHT: %g, %d\n", phase_m, sound_v_left[sound_no]);
	}

	// if (SampleRate != 44100 || NChannels != 2) {
	// 	SampleRate = 44100;
	// 	NChannels = 2;
	// 	SystemMode = MODE_SAMPLERATE;
	// 	while (SystemMode != MODE_RUN) {}
	// }

	audio_state = P_SOUND;

    SDL_UnlockAudioDevice(1);
	return kOk;
}

MmResult audio_play_tone(float f_left, float f_right, int64_t duration, const char *interrupt) {
    if (!audio_initialised) {
        MmResult result = audio_init();
        if (FAILED(result)) return result;
    }

    SDL_LockAudioDevice(1);

    // if(audio_state == P_TONE || audio_state == P_PAUSE_TONE) audio_state =
    // P_PAUSE_TONE;//StopAudio();                 // stop the current tone

    if (!(audio_state == P_NOTHING || audio_state == P_TONE ||
          audio_state == P_PAUSE_TONE)) {
        SDL_UnlockAudioDevice(1);
        return kSoundInUse;
    }

    if (f_left < 0.0 || f_left > 20000.0 || f_right < 0.0 || f_right > 20000.0) {
        SDL_UnlockAudioDevice(1);
        return kSoundInvalidFrequency;
    }

    if (duration == 0) {
        SDL_UnlockAudioDevice(1);
        return kOk;
    }

//    uint64_t PlayDuration = 0xffffffffffffffff;  // default is to play forever
//    int x;

    uint64_t play_duration = duration == -1 ? 0xFFFFFFFFFFFFFFFF : PWM_FREQ * duration / 1000;

    // TODO:
    // WAVInterrupt = GetIntAddress(argv[6]);  // get the interrupt location
    // InterruptUsed = true;

    if (play_duration != 0xffffffffffffffff && f_left >= 10.0) {
        float hw = ((float)PWM_FREQ / f_left);
        int x = (int)((float)(play_duration) / hw) + 1;
        play_duration = (uint64_t)((float)x * hw);
    }
    PhaseM_left = f_left / (float)PWM_FREQ * 4096.0f;
    PhaseM_right = f_right / (float)PWM_FREQ * 4096.0f;
    // WAV_fnbr = 0;
    // if (SampleRate != 44100 || NChannels != 2) {
    //     SampleRate = 44100;
    //     NChannels = 2;
    //     SystemMode = MODE_SAMPLERATE;
    //     while (SystemMode != MODE_RUN) {
    //     }
    // }
    SoundPlay = play_duration;
    audio_state = P_TONE;

    SDL_UnlockAudioDevice(1);
    return kOk;
}

MmResult audio_resume() {
    if (!audio_initialised) {
        MmResult result = audio_init();
        if (FAILED(result)) return result;
    }

    SDL_LockAudioDevice(1);

    MmResult result = kOk;
    switch (audio_state) {
        case P_PAUSE_TONE:
            audio_state = P_TONE;
            break;
        case P_PAUSE_FLAC:
            audio_state = P_FLAC;
            break;
        case P_PAUSE_SOUND:
            audio_state = P_SOUND;
            break;
        case P_PAUSE_MOD:
            audio_state = P_MOD;
            break;
        case P_PAUSE_WAV:
            audio_state = P_WAV;
            break;
        case P_PAUSE_MP3:
            audio_state = P_MP3;
            break;
        default:
            result = kAudioNothingToResume;
            break;
    }

    SDL_UnlockAudioDevice(1);
    return result;
}

void audio_service_buffers() {
    if (swingbuf != nextbuf) {
        if (audio_state == P_MOD) {
            char *buf = (nextbuf == 1) ? sbuff1 : sbuff2;
            hxcmod_fillbuffer(&audio_mod_context, (msample*)buf, WAV_BUFFER_SIZE / 4, NULL,
                              audio_mod_noloop ? 1 : 0);
            bcount[nextbuf] = WAV_BUFFER_SIZE / 2;
            nextbuf = swingbuf;
        }
    }
}

MmResult audio_set_volume(uint8_t left, uint8_t right) {
    fFilterVolumeL = (float)mapping[left] / 2001.0f;
    fFilterVolumeR = (float)mapping[right] / 2001.0f;
    return kOk;
}
