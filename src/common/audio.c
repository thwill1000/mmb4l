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
#include "interrupt.h"
#include "memory.h"
#include "mmresult.h"
#include "path.h"
#include "utility.h"
#define DR_FLAC_IMPLEMENTATION
#include "../third_party/dr_flac.h"
#define DR_MP3_IMPLEMENTATION
#include "../third_party/dr_mp3.h"
#define DR_WAV_IMPLEMENTATION
#include "../third_party/dr_wav.h"
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
#define LEFT_CHANNEL       0
#define RIGHT_CHANNEL      1
#define TONE_VOLUME        100

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

typedef struct {
    uint64_t byte_count;
    char *data;
} AudioBuffer;

static const char* NO_ERROR = "";

static bool audio_initialised = false;
static AudioState audio_state = P_NOTHING;

// Variables used by PLAY SOUND and PLAY TONE.
static const uint16_t *sound_mode[2][MAXSOUNDS] = {
    { null_table, null_table, null_table, null_table },
    { null_table, null_table, null_table, null_table }
};
static float audio_phase_ac[2][MAXSOUNDS] = {0};
static float audio_phase_m[2][MAXSOUNDS] = {0};
static int audio_sound_volume[2][MAXSOUNDS] = {0};
static uint64_t audio_tone_duration;

// Variables used by PLAY FLAC, PLAY MOD, PLAY_MP3 and PLAY_WAV
static int audio_fnbr = 0;
// static uint64_t bcount[3] = {0, 0, 0};  // Number of bytes(?) of data in audio buffer.
//                                         // I believe element 0 is ignored.
static float audio_filter_volume[2] = { 1.0f, 1.0f };
static int nextbuf = 0;  // Index of the buffer to fill.
static bool audio_file_finished = true;
static uint64_t ppos = 0;  // Playing position in the currently playing buffer.
static AudioBuffer audio_buf[2] = { 0 };
//static AudioBuffer *audio_play_buf = NULL;
//static AudioBuffer *audio_fill_buf = NULL;
//static char *sbuff1 = NULL;
//static char *sbuff2 = NULL;
static int swingbuf = 0;  // Index of the buffer to play.
static drwav audio_wav_struct;
static drmp3 audio_mp3_struct;
static drflac* audio_flac_struct = NULL;
static uint64_t audio_file_size;
static SDL_AudioSpec audio_current_spec;
static int audio_dummy_data;

// Variables used by PLAY MODFILE
static char* audio_modbuff = NULL;
static modcontext audio_mod_context = { 0 };
static bool audio_mod_noloop = false;

// static uint64_t bcounte[3] = {0, 0, 0};
//static AudioState audio_statee = P_NOTHING;
//static int mono;
//static int nextbufe = 0;
//static float audio_phase_m[2] = { 0.0f, 0.0f };
//static float audio_phase_ac[2] = { 0.0f, 0.0f };
//static bool audio_file_finishede = true;
//static uint64_t ppose = 0;
//static char *sbuff1e = NULL;
//static char *sbuff2e = NULL;

//static int swingbufe = 0;

static void audio_callback(void *userdata, Uint8 *stream, int len);

static void* audio_malloc(size_t sz, void* pUserData) {
    return GetMemory(sz);
}

static void* audio_realloc(void* p, size_t sz, void* pUserData) {
    return ReAllocMemory((p), (sz));
}

static void audio_free_memory(void* p, void* pUserData) {
    // FreeMemorySafe((void**)(&p));
    FreeMemory(p);
}

static size_t audio_on_read(void* pUserData, char* pBufferOut, size_t bytesToRead) {
    unsigned int nbr;
    nbr = fread(pBufferOut, 1, bytesToRead, file_table[audio_fnbr].file_ptr);
    return nbr;
}

static drmp3_bool32 audio_on_seek(void* pUserData, int offset, drmp3_seek_origin origin) {
    if (origin == drmp3_seek_origin_start) {
        fseek(file_table[audio_fnbr].file_ptr, offset, SEEK_SET);
    } else {
        fseek(file_table[audio_fnbr].file_ptr, offset, SEEK_CUR);
    }
    return 1;
}

/** Configures the SDL Audio sample rate and number of channels. */
static MmResult audio_configure(int sample_rate, int num_channels) {
    // printf("audio_configure()\n");
    // printf("  sample_rate = %d\n", sample_rate);
    // printf("  num_channels = %d\n", num_channels);

    if (audio_initialised
        && audio_current_spec.freq == sample_rate
        && audio_current_spec.channels == num_channels) return kOk;

    SDL_CloseAudio();
    audio_current_spec = (SDL_AudioSpec) {
      .format = AUDIO_F32,
      .channels = num_channels,
      .freq = sample_rate,
      .samples = BUFFER_SIZE,
      .callback = audio_callback,
    };

    if (FAILED(SDL_OpenAudio(&audio_current_spec, NULL))) {
        return kAudioApiError;
    }

    return kOk;
}

MmResult audio_init() {
    if (audio_initialised) return kOk;
    MmResult result = events_init();

    if (SUCCEEDED(result)) {
        result = audio_configure(AUDIO_SAMPLE_RATE, 2);
    }

    if (SUCCEEDED(result)) {
        audio_tables_init();
        audio_initialised = true;
    }

    return result;
}

const char *audio_last_error() {
    const char* emsg = SDL_GetError();
    return emsg && *emsg ? emsg : NO_ERROR;
}

static void audio_free_buffers() {
    for (int i = 0; i < 2; ++i) {
        FreeMemory((void *) audio_buf[i].data);
        audio_buf[i].data = NULL;
        audio_buf[i].byte_count = 0;
    }
}

static MmResult audio_alloc_buffers(size_t size) {
    audio_free_buffers();
    for (int i = 0; i < 2; ++i) audio_buf[i].data = (char*) GetMemory(size);
    return kOk;
}

MmResult audio_close(bool all) {
    if (!audio_initialised) return kOk;

    SDL_LockAudioDevice(1);

    // int was_playing = audio_state;
    audio_state = P_NOTHING;
    audio_free_buffers();
    // memset(bcount, 0, sizeof(bcount));
    swingbuf = nextbuf = 0;
    audio_file_finished = false;
    //memset(bcounte, 0, sizeof(bcounte));
    //swingbufe = nextbufe = 0;
    //audio_file_finishede = false;
    // WAVInterrupt = NULL;
    // if (was_playing == P_MP3 || was_playing == P_PAUSE_MP3) drmp3_uninit(&mymp3);
    // if (was_playing == P_FLAC || was_playing == P_PAUSE_FLAC) FreeMemorySafe((void **)&myflac);
    // ForceFileClose(WAV_fnbr);
    // FreeMemorySafe((void **)&modbuff);
    // for (int i = 0; i < 2; ++i) {
    //     FreeMemory((void *) (audio_buf[i].data));
    //     audio_buf[i].byte_count = 0;
    //     audio_buf[i].data = NULL;
    // }
    //FreeMemory((void *) sbuff1); sbuff1 = NULL; // FreeMemorySafe((void **)&sbuff1);
    //FreeMemory((void *) sbuff2); sbuff2 = NULL; // FreeMemorySafe((void **)&sbuff2);
    //FreeMemory((void *) sbuff1e); sbuff1e = NULL; // FreeMemorySafe((void **)&sbuff1e);
    //FreeMemory((void *) sbuff2e); sbuff2e = NULL; // FreeMemorySafe((void **)&sbuff2e);
    // FreeMemorySafe((void **)&mymp3);
    // memset(&mywav, 0, sizeof(drwav));
    // if (all) {
    //     FreeMemorySafe((void **)&alist);
    //     trackstoplay = 0;
    //     trackplaying = 0;
    // }
    audio_tone_duration = 0;
    ppos = 0;
    for (int snd = 0; snd < MAXSOUNDS; ++snd) {
        for (int channel = LEFT_CHANNEL; channel <= RIGHT_CHANNEL; ++channel) {
            audio_phase_m[channel][snd] = 0.0f;
            audio_phase_ac[channel][snd] = 0.0f;
            sound_mode[channel][snd] = null_table;
        }
    }

    // interrupt_disable(kInterruptAudio);

    SDL_UnlockAudioDevice(1);

    return kOk;
}

static float audio_callback_tone(int channel) {
    if (audio_tone_duration <= 0) {
        interrupt_fire(kInterruptAudio);
        audio_close(1);
        return 0.0f;
    } else {
        audio_tone_duration--;
        const int v = (((sine_table[(int)audio_phase_ac[0][channel]] - 2000) * mapping[TONE_VOLUME]) / 2000) + 2000;
        audio_phase_ac[0][channel] += audio_phase_m[0][channel];
        if (audio_phase_ac[0][channel] >= 4096.0) audio_phase_ac[0][channel] -= 4096.0;
        return ((float)v - 2000.0f) / 2000.0f;
    }
}

static float audio_callback_mod(int channel) {
    float value = 0, valuee = 0;

    if (swingbuf) {  // buffer is primed
        const int buf_idx = swingbuf - 1;
        int16_t *buf = (int16_t *) audio_buf[buf_idx].data;
        if (ppos < audio_buf[buf_idx].byte_count) {
            value = (float)buf[ppos++] / 32768.0f * audio_filter_volume[channel];
        }

        // If we are now at the end of the buffer ...
        if (ppos == audio_buf[buf_idx].byte_count) {
            if (audio_buf[buf_idx == 0 ? 1 : 0].byte_count != 0) {
                // If the alternative buffer is not empty then swap buffer.
                audio_buf[buf_idx].byte_count = 0;
                swingbuf = swingbuf == 1 ? 2 : 1;
            } else {
                // Alternative buffer is empty so replay current buffer.
                nextbuf = swingbuf == 1 ? 2 : 1;
            }
            ppos = 0;
        }
    }

    return value + valuee;
}

// Callback for FLAC, MP3 and WAV files.
static float audio_callback_track(int channel) {
    float value = 0;

    if (swingbuf) {  // buffer is primed
        const int buf_idx = swingbuf - 1;
        float *flacbuff = (float *) audio_buf[buf_idx].data;

        if (ppos < audio_buf[buf_idx].byte_count) {
            value = flacbuff[ppos++] * audio_filter_volume[channel];
        }

        if (ppos == audio_buf[buf_idx].byte_count) {
            // Reached end of currently playing buffer.
            const int psave = ppos;
            audio_buf[buf_idx].byte_count = 0;
            ppos = 0;
            swingbuf = (swingbuf == 1) ? 2 : 1;
            if (audio_buf[swingbuf - 1].byte_count == 0 && !audio_file_finished) {
                // Nothing ready yet so flip back.
                swingbuf = (swingbuf == 1) ? 2 : 1;
                nextbuf = (swingbuf == 1) ? 2 : 1;
                audio_buf[buf_idx].byte_count = psave;
                ppos = 0;
            }
        }
    }

    return value;
}

static float audio_callback_sound(int channel) {
    static int noisedwell[2][MAXSOUNDS] = { 0 };
    static uint32_t noise[2][MAXSOUNDS] = { 0 };

    int volume = 0;
    int delta;

    for (int i = 0; i < MAXSOUNDS; ++i) {
        if (sound_mode[channel][i] == null_table) continue;

        if (sound_mode[channel][i] != white_noise_table) {
            audio_phase_ac[channel][i] = audio_phase_ac[channel][i] + audio_phase_m[channel][i];
            if (audio_phase_ac[channel][i] >= 4096.0) audio_phase_ac[channel][i] -= 4096.0;
            delta = sound_mode[channel][i][(int)audio_phase_ac[channel][i]];
        } else {
            if (noisedwell[channel][i] <= 0) {
                noisedwell[channel][i] = (int)audio_phase_m[channel][i];
                noise[channel][i] = rand() % 3700 + 100;
            }
            if (noisedwell[channel][i]) noisedwell[channel][i]--;
            delta = noise[channel][i];
        }
        delta = (delta - 2000) * mapping[audio_sound_volume[channel][i]] / 2000;
        volume += delta;
    }

    return ((float)volume) / 2000.0f;
}


// For the moment len will always be 4 bytes (1 sample) for mono or 8 bytes (2 samples) for stereo.
static void audio_callback(void *userdata, Uint8 *stream, int len) {
    // static int counter = 0;
    float *fstream = (float *)stream;
    for (int i = 0; i < BUFFER_SIZE; i += 2) { // The 2 is irrelevant, BUFFER_SIZE = 1.
        switch (audio_state) {
            case P_TONE:
                fstream[i] = audio_callback_tone(0);
                if (len == 8) fstream[i + 1] = audio_callback_tone(1);
                break;
            case P_MOD:
                fstream[i] = audio_callback_mod(0);
                if (len == 8) fstream[i + 1] = audio_callback_mod(1);
                break;
            case P_MP3:
            case P_WAV:
            case P_FLAC:
                fstream[i] = audio_callback_track(0);
                if (len == 8) fstream[i + 1] = audio_callback_track(1);
                break;
            case P_SOUND:
                fstream[i] = audio_callback_sound(0);
                if (len == 8) fstream[i + 1] = audio_callback_sound(1);
                break;
            default:
                fstream[i] = 0.0f;
                if (len == 8) fstream[i + 1] = 0.0f;
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

static bool audio_is_valid_sample_rate(unsigned sample_rate) {
    return sample_rate == 8000
            || sample_rate == 16000
            || sample_rate == 22050
            || sample_rate == 44100
            || sample_rate == 48000;
}

MmResult audio_play_effect(const char *filename, const char *interrupt) {
    return kOk;
}

/**
 * Opens an audio file.
 *
 * @param  filename   Name of the file, with optional extension.
 * @param  extension  Extension to append if missing.
 */
static MmResult audio_open_file(const char *filename, const char *extension) {
    MmResult result = kOk;

    char filename_with_ext[STRINGSIZE];
    cstring_cpy(filename_with_ext, filename, STRINGSIZE);
    if (!path_has_suffix(filename_with_ext, extension, true)) {
        // TODO: Handle file-extension in a case-insensitive manner.
        result = cstring_cat(filename_with_ext, extension, STRINGSIZE);
        if (FAILED(result)) return result;
    }

    audio_fnbr = file_find_free();
    return file_open(filename_with_ext, "rb", audio_fnbr);
}

MmResult audio_play_modfile(const char *filename, unsigned sample_rate, const char *interrupt) {
    MmResult result = kOk;

    if (!audio_initialised) {
        result = audio_init();
        if (FAILED(result)) return result;
    }

    SDL_PauseAudio(1);

    if (!audio_is_valid_sample_rate(sample_rate)) {
        result = kAudioInvalidSampleRate;
    }

    if (SUCCEEDED(result) && audio_state != P_NOTHING) {
        result = kSoundInUse;
    }

    if (SUCCEEDED(result)) {
        result = audio_configure(sample_rate, 2);
    }

    if (SUCCEEDED(result)) {
        result = audio_open_file(filename, ".MOD");
    }

    // Read the file into 'audio_modbuff'
    // TODO: If file_lof() or file_read() report error this will leave audio device paused.
    int size = 0;
    if (SUCCEEDED(result)) {
        size = file_lof(audio_fnbr);
        audio_modbuff = (char *)GetMemory(size + 256);
        file_read(audio_fnbr, audio_modbuff, size);
        result = file_close(audio_fnbr);
    }

    if (SUCCEEDED(result)) {
        audio_alloc_buffers(WAV_BUFFER_SIZE);

        hxcmod_init(&audio_mod_context);
        hxcmod_setcfg(&audio_mod_context, sample_rate, 1, 1);
        hxcmod_load(&audio_mod_context, (void*)audio_modbuff, size);
        hxcmod_fillbuffer(&audio_mod_context, (msample*)audio_buf[0].data, WAV_BUFFER_SIZE / 4, NULL,
                          audio_mod_noloop ? 1 : 0);
        audio_buf[0].byte_count = WAV_BUFFER_SIZE / 2;
        audio_buf[1].byte_count = 0;
        swingbuf = 1;
        nextbuf = 2;
        ppos = 0;
        audio_file_finished = false;
        audio_state = P_MOD;
        // TODO: Get MOD interrupt working.
        // audio_mod_noloop = interrupt != NULL;
        interrupt_enable(kInterruptAudio, interrupt);
    }

    SDL_PauseAudio(0);
    return kOk;
}

MmResult audio_play_modsample(uint8_t sample_num, uint8_t channel_num, uint8_t volume,
                              unsigned sample_rate) {
    if (!audio_initialised) {
        MmResult result = audio_init();
        if (FAILED(result)) return result;
    }

    if (!audio_is_valid_sample_rate(sample_rate)) return kAudioInvalidSampleRate;

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

static inline bool audio_is_valid_frequency(MMFLOAT f) {
    // Should it be < 1.0 ? "Valid is 1Hz to 20KHz"
    return f >= 0.0 && f <= 20000.0;
}

MmResult audio_play_sound(uint8_t sound_no, Channel channel, SoundType type, float frequency,
                          uint8_t volume) {
    MmResult result = kOk;

    if (!audio_initialised) {
        result = audio_init();
        if (FAILED(result)) return result;
    }

    SDL_PauseAudio(1);

    if (audio_state != P_NOTHING && audio_state != P_SOUND && audio_state != P_PAUSE_SOUND) {
        result = kSoundInUse;
    }

    if (SUCCEEDED(result) && !audio_is_valid_frequency(frequency)) {
        result = kSoundInvalidFrequency;
    }

    const uint16_t *previous[2] = { sound_mode[LEFT_CHANNEL][sound_no], sound_mode[RIGHT_CHANNEL][sound_no] } ;

    sound_mode[LEFT_CHANNEL][sound_no] = null_table;
    sound_mode[RIGHT_CHANNEL][sound_no] = null_table;

    for (int c = LEFT_CHANNEL /* 0 */; c <= RIGHT_CHANNEL /* 1 */; ++c) {
        if (c == LEFT_CHANNEL && !(channel & kChannelLeft)) {
            continue;
        } else if (c == RIGHT_CHANNEL && !(channel & kChannelRight)) {
            continue;
        }
        switch (type) {
            case kSoundTypeSine:
                sound_mode[c][sound_no] = sine_table;
                break;
            case kSoundTypeSquare:
                sound_mode[c][sound_no] = square_table;
                break;
            case kSoundTypeTriangular:
                sound_mode[c][sound_no] = triangular_table;
                break;
            case kSoundTypeSawTooth:
                sound_mode[c][sound_no] = saw_tooth_table;
                break;
            case kSoundTypePeriodicNoise:
                sound_mode[c][sound_no] = periodic_noise_table;
                break;
            case kSoundTypeWhiteNoise:
                sound_mode[c][sound_no] = white_noise_table;
                break;
            case kSoundTypeNull:
                // Already set to null_table.
                break;
            default:
                result = kInternalFault;
        }
    }

    if (SUCCEEDED(result)) {
        result = audio_configure(44100, 2);
    }

    if (SUCCEEDED(result)) {
        for (int c = LEFT_CHANNEL /* 0 */; c <= RIGHT_CHANNEL /* 1 */; ++c) {
            if (c == LEFT_CHANNEL && !(channel & kChannelLeft)) {
                continue;
            } else if (c == RIGHT_CHANNEL && !(channel & kChannelRight)) {
                continue;
            }

            if (previous[c] == null_table) audio_phase_ac[c][sound_no] = 0.0f;
            audio_phase_m[c][sound_no] = sound_mode[c][sound_no] == white_noise_table
                    ? frequency
                    : frequency / (float)PWM_FREQ * 4096.0f;
            audio_sound_volume[c][sound_no] = (volume * 41) / 25;
        }

        audio_state = P_SOUND;
    }

    SDL_PauseAudio(0);
    return kOk;
}

MmResult audio_play_tone(float f_left, float f_right, int64_t duration, const char *interrupt) {
    MmResult result = kOk;

    if (!audio_initialised) {
        result = audio_init();
        if (FAILED(result)) return result;
    }

    SDL_PauseAudio(1);

    // if(audio_state == P_TONE || audio_state == P_PAUSE_TONE) audio_state =
    // P_PAUSE_TONE;//StopAudio();                 // stop the current tone

    if (audio_state != P_NOTHING && audio_state != P_TONE && audio_state != P_PAUSE_TONE) {
        result = kSoundInUse;
    }

    if (SUCCEEDED(result) && !(audio_is_valid_frequency(f_left) && audio_is_valid_frequency(f_right))) {
        result = kSoundInvalidFrequency;
    }

    if (SUCCEEDED(result) && duration == 0) {
        SDL_PauseAudio(0);
        return kOk;
    }

    if (SUCCEEDED(result)) {
        result = audio_configure(44100, 2);
    }

    if (SUCCEEDED(result)) {
        uint64_t play_duration = duration == -1 ? 0xFFFFFFFFFFFFFFFF : PWM_FREQ * duration / 1000;

        if (play_duration != 0xffffffffffffffff && f_left >= 10.0) {
            float hw = ((float)PWM_FREQ / f_left);
            int x = (int)((float)(play_duration) / hw) + 1;
            play_duration = (uint64_t)((float)x * hw);
        }
        audio_phase_m[0][LEFT_CHANNEL] = f_left / (float)PWM_FREQ * 4096.0f;
        audio_phase_m[0][RIGHT_CHANNEL] = f_right / (float)PWM_FREQ * 4096.0f;

        audio_tone_duration = play_duration;
        audio_state = P_TONE;

        interrupt_enable(kInterruptAudio, interrupt);
    }

    SDL_PauseAudio(0);
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

static MmResult audio_play_flac_internal(const char *filename) {
    MmResult result = audio_open_file(filename, ".FLAC");

    if (SUCCEEDED(result)) {
        drflac_allocation_callbacks allocationCallbacks;
        allocationCallbacks.pUserData = &audio_dummy_data;
        allocationCallbacks.onMalloc = audio_malloc;
        allocationCallbacks.onRealloc = audio_realloc;
        allocationCallbacks.onFree = audio_free_memory;

        audio_flac_struct = drflac_open((drflac_read_proc) audio_on_read,
                                        (drflac_seek_proc) audio_on_seek, &audio_dummy_data,
                                        &allocationCallbacks);
        if (!audio_flac_struct) result =  kAudioFlacInitialisationFailed;
    }

    if (SUCCEEDED(result)) {
        result = audio_alloc_buffers(WAV_BUFFER_SIZE*4);
    }

    if (SUCCEEDED(result)) {
        result = audio_configure(audio_flac_struct->sampleRate, 2);
    }

    if (SUCCEEDED(result)) {
        audio_buf[0].byte_count = drflac_read_pcm_frames_f32(audio_flac_struct, WAV_BUFFER_SIZE / 2,
                                                             (float*) audio_buf[0].data)
                                  * audio_flac_struct->channels;
        audio_file_size = audio_buf[0].byte_count;
        swingbuf = 1;
        nextbuf = 2;
        ppos = 0;
        audio_file_finished = false;
        audio_state = P_FLAC;
    }

    return result;
}

MmResult audio_play_flac(const char *filename, const char *interrupt) {
    MmResult result = kOk;

    if (!audio_initialised) {
        result = audio_init();
        if (FAILED(result)) return result;
    }

    SDL_PauseAudio(1);

    if (audio_state != P_NOTHING) result = kSoundInUse;
    if (SUCCEEDED(result)) result = audio_play_flac_internal(filename);
    if (SUCCEEDED(result)) interrupt_enable(kInterruptAudio, interrupt);

    SDL_PauseAudio(0);
    return result;
}

static MmResult audio_play_mp3_internal(const char *filename) {
    MmResult result = audio_open_file(filename, ".MP3");

    if (SUCCEEDED(result)) {
        drmp3_allocation_callbacks allocationCallbacks;
        allocationCallbacks.pUserData = &audio_dummy_data;
        allocationCallbacks.onMalloc = audio_malloc;
        allocationCallbacks.onRealloc = audio_realloc;
        allocationCallbacks.onFree = audio_free_memory;

        if (drmp3_init(&audio_mp3_struct, (drmp3_read_proc) audio_on_read,
                       (drmp3_seek_proc) audio_on_seek, NULL, &allocationCallbacks) == DRMP3_FALSE) {
            result = kAudioMp3InitialisationFailed;
        }
    }

    if (SUCCEEDED(result)) {
        result = audio_alloc_buffers(WAV_BUFFER_SIZE*4);
    }

    if (SUCCEEDED(result)) {
        result = audio_configure(audio_mp3_struct.sampleRate, 2);
    }

    if (SUCCEEDED(result)) {
        // audio_play_buf = audio_buf[0];
        // audio_fill_buf = audio_buf[1];
        audio_buf[0].byte_count = drmp3_read_pcm_frames_f32(&audio_mp3_struct, WAV_BUFFER_SIZE / 2,
                                                            (float*) audio_buf[0].data)
                                  * audio_mp3_struct.channels;
        audio_file_size = audio_buf[0].byte_count;
        swingbuf = 1;
        nextbuf = 2;
        ppos = 0;
        audio_file_finished = false;
        audio_state = P_MP3;
    }

    return result;
}

MmResult audio_play_mp3(const char *filename, const char *interrupt) {
    MmResult result = kOk;

    if (!audio_initialised) {
        result = audio_init();
        if (FAILED(result)) return result;
    }

    SDL_PauseAudio(1);

    if (audio_state != P_NOTHING) result = kSoundInUse;
    if (SUCCEEDED(result)) result = audio_play_mp3_internal(filename);
    if (SUCCEEDED(result)) interrupt_enable(kInterruptAudio, interrupt);

    SDL_PauseAudio(0);
    return result;
}

static MmResult audio_play_wav_internal(const char *filename) {
    MmResult result = audio_open_file(filename, ".WAV");

    if (SUCCEEDED(result)) {
        drwav_allocation_callbacks allocationCallbacks;
        allocationCallbacks.pUserData = &audio_dummy_data;
        allocationCallbacks.onMalloc = audio_malloc;
        allocationCallbacks.onRealloc = audio_realloc;
        allocationCallbacks.onFree = audio_free_memory;

        if (drwav_init(&audio_wav_struct, (drwav_read_proc) audio_on_read,
                       (drwav_seek_proc) audio_on_seek, NULL, &allocationCallbacks) == DRWAV_FALSE)
            result =  kAudioWavInitialisationFailed;
    }

    if (SUCCEEDED(result)) {
        result = audio_alloc_buffers(WAV_BUFFER_SIZE*4);
    }

    if (SUCCEEDED(result)) {
        result = audio_configure(audio_wav_struct.sampleRate, audio_wav_struct.channels);
    }

    if (SUCCEEDED(result)) {
        audio_buf[0].byte_count = drwav_read_pcm_frames_f32(&audio_wav_struct, WAV_BUFFER_SIZE / 2,
                                                            (float*) audio_buf[0].data)
                                  * audio_wav_struct.channels;
        audio_file_size = audio_buf[0].byte_count;
        swingbuf = 1;
        nextbuf = 2;
        ppos = 0;
        audio_file_finished = false;
        audio_state = P_WAV;
    }

    return result;
}

MmResult audio_play_wav(const char *filename, const char *interrupt) {
    MmResult result = kOk;

    if (!audio_initialised) {
        result = audio_init();
        if (FAILED(result)) return result;
    }

    SDL_PauseAudio(1);

    if (audio_state != P_NOTHING) result = kSoundInUse;
    if (SUCCEEDED(result)) result = audio_play_wav_internal(filename);
    if (SUCCEEDED(result)) interrupt_enable(kInterruptAudio, interrupt);

    SDL_PauseAudio(0);
    return result;
}

MmResult audio_background_tasks() {
    switch (audio_state) {
        case P_FLAC:
        case P_MOD:
        case P_MP3:
        case P_WAV:
            break;
        default:
            // Nothing to do.
            return kOk;
    }

    if (swingbuf != nextbuf) {
        const int buf_idx = (nextbuf == 1) ? 0 : 1;
        char *buf = audio_buf[buf_idx].data;
        switch (audio_state) {
            case P_FLAC: {
                audio_buf[buf_idx].byte_count = drflac_read_pcm_frames_f32(
                    audio_flac_struct, WAV_BUFFER_SIZE / 2, (float*) buf)
                    * audio_flac_struct->channels;
                audio_file_size = audio_buf[buf_idx].byte_count;
                break;
            }

            case P_MOD: {
                hxcmod_fillbuffer(&audio_mod_context, (msample*)buf, WAV_BUFFER_SIZE / 4, NULL,
                                 audio_mod_noloop ? 1 : 0);
                audio_buf[buf_idx].byte_count = WAV_BUFFER_SIZE / 2;
                break;
            }

            case P_WAV: {
                audio_buf[buf_idx].byte_count = drwav_read_pcm_frames_f32(
                    &audio_wav_struct, WAV_BUFFER_SIZE / 2, (float*) buf)
                    * audio_wav_struct.channels;
                audio_file_size = audio_buf[buf_idx].byte_count;
                break;
            }

            case P_MP3: {
                audio_buf[buf_idx].byte_count = drmp3_read_pcm_frames_f32(
                    &audio_mp3_struct, WAV_BUFFER_SIZE / 2, (float*) buf)
                    * audio_mp3_struct.channels;
                audio_file_size = audio_buf[buf_idx].byte_count;
                break;
            }

            default:
                break;
        }
        nextbuf = swingbuf;
    }

    // Check if FLAC, MP3 or WAV playback has finished.
    if (audio_file_size <= 0) {
        switch (audio_state) {
            case P_FLAC:
                (void) drflac_close(audio_flac_struct);
                break;
            case P_MP3:
                (void) drmp3_uninit(&audio_mp3_struct);
                break;
            case P_WAV:
                audio_file_finished = true;
                break;
            default:
                break;
        }
    }

    if (audio_file_finished) {
        if (!(audio_buf[0].byte_count || audio_buf[1].byte_count)) {
            switch (audio_state) {
                case P_FLAC:
                    (void) drflac_close(audio_flac_struct);
                    break;
                case P_MP3:
                    (void) drmp3_uninit(&audio_mp3_struct);
                    break;
                case P_WAV:
                    (void) drwav_uninit(&audio_wav_struct);
                    break;
                default:
                    printf("UNEXPECTED2\n");
                    break;
            }
            audio_free_buffers();
            // FreeMemory((void *) alist);
            audio_state = P_NOTHING;
            (void) file_close(audio_fnbr);
            audio_file_finished = false;
            interrupt_fire(kInterruptAudio);
            // memset(&mywav, 0, sizeof(drwav));
        } else {
            printf("UNEXPECTED\n");
        }
    }

    return kOk;
}

MmResult audio_set_volume(uint8_t left, uint8_t right) {
    audio_filter_volume[LEFT_CHANNEL] = (float) mapping[left] / 2001.0f;
    audio_filter_volume[RIGHT_CHANNEL] = (float) mapping[right] / 2001.0f;
    return kOk;
}
