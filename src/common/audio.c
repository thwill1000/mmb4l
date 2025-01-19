/*-*****************************************************************************

MMBasic for Linux (MMB4L)

audio.c

Copyright 2021-2025 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

#include <SDL.h>
#include <assert.h>
#include <dirent.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../third_party/dr_flac.h"
#include "../third_party/dr_mp3.h"
#include "../third_party/dr_wav.h"
#include "../third_party/hxcmod.h"
#include "audio_tables.h"
#include "console.h"
#include "cstring.h"
#include "error.h"
#include "events.h"
#include "file.h"
#include "interrupt.h"
#include "memory.h"
#include "mmresult.h"
#include "path.h"
#include "utility.h"

#define AUDIO_SAMPLE_RATE 44100UL
#define WAV_BUFFER_SIZE 16384
#define LEFT_CHANNEL 0
#define RIGHT_CHANNEL 1
#define MAX_TRACKS 100
#define TONE_VOLUME 100

#if 0
#define LOCK_AUDIO(s)    printf("%s: lock audio\n", s); SDL_LockAudio()
#define UNLOCK_AUDIO(s)  printf("%s: unlock audio\n", s); SDL_UnlockAudio()
#else
#define LOCK_AUDIO(s)    SDL_LockAudio()
#define UNLOCK_AUDIO(s)  SDL_UnlockAudio()
#endif

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

static const char *NO_ERROR = "";

static bool audio_initialised = false;
static AudioState audio_state = P_NOTHING;

////////////////////////////////////////////////////////////////////////////////
// Variables used by PLAY SOUND and PLAY TONE.
////////////////////////////////////////////////////////////////////////////////
static const uint16_t *audio_sound[2][MAXSOUNDS] = {
    {null_table, null_table, null_table, null_table},
    {null_table, null_table, null_table, null_table}};
static float audio_phase_ac[2][MAXSOUNDS] = {0};
static float audio_phase_m[2][MAXSOUNDS] = {0};
static int audio_sound_volume[2][MAXSOUNDS] = {0};
static uint64_t audio_tone_duration;
static char audio_track_list[MAX_TRACKS][STRINGSIZE];
static unsigned audio_track_current = 0;

////////////////////////////////////////////////////////////////////////////////
// Variables used by PLAY FLAC, PLAY MOD, PLAY_MP3 and PLAY_WAV
////////////////////////////////////////////////////////////////////////////////
static int audio_fnbr = -1;
static float audio_filter_volume[2] = {1.0f, 1.0f};

/** Playing position in the currently playing buffer. */
static uint64_t audio_pos = 0;

static AudioBuffer audio_buf[2] = {0};
static AudioBuffer *audio_fill_buf = NULL;
static AudioBuffer *audio_play_buf = NULL;
static drwav audio_wav_struct;
static drmp3 audio_mp3_struct;
static drflac *audio_flac_struct = NULL;
static SDL_AudioSpec audio_current_spec;
static int audio_dummy_data;

////////////////////////////////////////////////////////////////////////////////
// Variables used by PLAY MODFILE
////////////////////////////////////////////////////////////////////////////////
static char *audio_mod_buf = NULL;
static modcontext audio_mod_context = {0};

/** Set true to continuously loop MOD file. */
static bool audio_mod_loop = true;

/** Sample rate of MOD file. */
static uint32_t audio_mod_sample_rate = 0;

////////////////////////////////////////////////////////////////////////////////
// Variables used by PLAY EFFECT
////////////////////////////////////////////////////////////////////////////////
static AudioState audio_effect_state = P_NOTHING;
static drwav audio_effect_struct;
static AudioBuffer audio_effect_buf[2] = {0};
static AudioBuffer *audio_effect_fill_buf = NULL;
static AudioBuffer *audio_effect_play_buf = NULL;
static uint64_t audio_effect_pos = 0;

static void audio_callback(void *userdata, Uint8 *stream, int len);

static MmResult audio_play_next_track();

static void *audio_malloc(size_t sz, void *pUserData) { return GetMemory(sz); }

static void *audio_realloc(void *p, size_t sz, void *pUserData) { return ReAllocMemory((p), (sz)); }

static void audio_free_memory(void *p, void *pUserData) { FreeMemory(p); }

static size_t audio_on_read(void *pUserData, char *pBufferOut, size_t bytesToRead) {
    unsigned int nbr;
    nbr = fread(pBufferOut, 1, bytesToRead, file_table[audio_fnbr].file_ptr);
    return nbr;
}

static drmp3_bool32 audio_on_seek(void *pUserData, int offset, drmp3_seek_origin origin) {
    if (origin == drmp3_seek_origin_start) {
        fseek(file_table[audio_fnbr].file_ptr, offset, SEEK_SET);
    } else {
        fseek(file_table[audio_fnbr].file_ptr, offset, SEEK_CUR);
    }
    return 1;
}

/**
 * Configures the SDL Audio sample rate and number of channels.
 *
 * This should only be called whilst holding the audio lock on audio device 1.
 * If the sample rate or number of channels has changed then the lock will be
 * released on audio device 1, it will be closed, a new device 1 created in its
 * place and a lock acquired on the new device 1.
 */
static MmResult audio_configure(int sample_rate, int num_channels) {
    // printf("audio_configure()\n");
    // printf("  sample_rate = %d\n", sample_rate);
    // printf("  num_channels = %d\n", num_channels);

    MmResult result = kOk;
    if (audio_current_spec.freq != sample_rate || audio_current_spec.channels != num_channels) {
        UNLOCK_AUDIO("audio_configure"); // Release lock on the old audio.

        SDL_CloseAudio();

        audio_current_spec = (SDL_AudioSpec){
            .format = AUDIO_F32,
            .channels = num_channels,
            .freq = sample_rate,
            .samples = 1,  // TODO: Support a bigger sample buffer.
            .callback = audio_callback,
        };

        if (mmb_options.audio) {
            if (FAILED(SDL_OpenAudio(&audio_current_spec, NULL))) result = kAudioApiError;
        }

        LOCK_AUDIO("audio_configure"); // Acquire lock on the new audio.
    }

    // A new device starts paused, this will resume it.
    // In the event we have not created a new device it is harmless to resume an existing unpaused
    // device.
    if (mmb_options.audio) SDL_PauseAudio(0);
    return result;
}

MmResult audio_init() {
    if (audio_initialised) return kOk;

    MmResult result = events_init();

    if (SUCCEEDED(result)) {
        audio_tables_init();
        audio_initialised = true;
    }

    if (SUCCEEDED(result)) {
        audio_current_spec = (SDL_AudioSpec){
            .format = AUDIO_F32,
            .channels = 2,
            .freq = AUDIO_SAMPLE_RATE,
            .samples = 1,  // TODO: Support a bigger sample buffer.
            .callback = audio_callback,
        };

        if (mmb_options.audio) {
            if (FAILED(SDL_OpenAudio(&audio_current_spec, NULL))) result = kAudioApiError;
        }
    }

    return result;
}

const char *audio_last_error() {
    const char *emsg = SDL_GetError();
    return emsg && *emsg ? emsg : NO_ERROR;
}

static void audio_free_buffers() {
    for (int i = 0; i < 2; ++i) {
        FreeMemory((void *)audio_buf[i].data);
        audio_buf[i].data = NULL;
        audio_buf[i].byte_count = 0;
    }
    audio_play_buf = NULL;
    audio_fill_buf = NULL;
    audio_pos = 0;
}

static MmResult audio_alloc_buffers(size_t size) {
    audio_free_buffers();
    for (int i = 0; i < 2; ++i) audio_buf[i].data = (char *)GetMemory(size);
    audio_play_buf = &audio_buf[0];
    audio_fill_buf = &audio_buf[1];
    return kOk;
}

static void audio_effect_free_buffers() {
    for (int i = 0; i < 2; ++i) {
        FreeMemory((void *)audio_effect_buf[i].data);
        audio_effect_buf[i].data = NULL;
        audio_effect_buf[i].byte_count = 0;
    }
    audio_effect_play_buf = NULL;
    audio_effect_fill_buf = NULL;
    audio_effect_pos = 0;
}

static MmResult audio_effect_alloc_buffers(size_t size) {
    audio_effect_free_buffers();
    for (int i = 0; i < 2; ++i) audio_effect_buf[i].data = (char *)GetMemory(size);
    audio_effect_play_buf = &audio_effect_buf[0];
    audio_effect_fill_buf = &audio_effect_buf[1];
    return kOk;
}

static void audio_free_mod_buf() {
    if (audio_mod_buf) FreeMemory((void *)audio_mod_buf);
    audio_mod_buf = NULL;
}

static void audio_alloc_mod_buf(size_t size) {
    audio_free_mod_buf();
    audio_mod_buf = (char *)GetMemory(size + 256);
}

static MmResult audio_close_file() {
    if (audio_fnbr != -1) {
        MmResult result = file_close(audio_fnbr);
        audio_fnbr = -1;
        return result;
    } else {
        return kOk;
    }
}

static void audio_clear_track_list() {
    for (int counter = 0; counter < 100; ++counter) {
        audio_track_list[counter][0] = '\0';
    }
    audio_track_current = -1;
}

static void audio_dump_track_list() {
    printf("-- START --\n");
    for (int i = 0; i < MAX_TRACKS; ++i) {
        if (audio_track_list[i][0]) {
            printf("[%d] %s\n", i, audio_track_list[i]);
        }
    }
    printf("-- END --\n");
}

static MmResult audio_fill_track_list(const char *filename, const char *extension) {
    audio_clear_track_list();

    char canonical[STRINGSIZE];
    ON_FAILURE_RETURN(path_get_canonical(filename, canonical, STRINGSIZE));
    char tmp[STRINGSIZE];

    // Check for a single file.
    {
        MmResult result = path_try_extension(canonical, extension, tmp, STRINGSIZE);
        if (SUCCEEDED(result)) {
            cstring_cpy(audio_track_list[0], tmp, STRINGSIZE);
            return kOk;
        } else if (result != kFileNotFound) {
            return result;
        }
    }

    // Treat 'canonical' as a directory to search.
    {
        errno = 0;
        const char *dirname = canonical;
        DIR *dir = opendir(dirname);
        if (!dir) return errno;
        struct dirent *ent;
        size_t counter = 0;
        while (counter != MAX_TRACKS && (ent = readdir(dir)) != NULL) {
            if (path_has_extension(ent->d_name, extension, true)) {
                if (FAILED(cstring_cpy(tmp, dirname, STRINGSIZE))) {
                    return kFilenameTooLong;
                }
                if (FAILED(path_append(tmp, ent->d_name, audio_track_list[counter++],
                                       STRINGSIZE))) {
                    return kFilenameTooLong;
                }
            }
        }
        closedir(dir);
    }

    return (MmResult) errno;
}

MmResult audio_stop() {
    if (!audio_initialised) return kOk;

    LOCK_AUDIO("audio_stop");

    (void)audio_close_file();
    audio_state = P_NOTHING;
    audio_effect_state = P_NOTHING;
    audio_free_buffers();
    audio_effect_free_buffers();
    audio_free_mod_buf();
    for (int snd = 0; snd < MAXSOUNDS; ++snd) {
        for (int channel = LEFT_CHANNEL; channel <= RIGHT_CHANNEL; ++channel) {
            audio_phase_m[channel][snd] = 0.0f;
            audio_phase_ac[channel][snd] = 0.0f;
            audio_sound[channel][snd] = null_table;
        }
    }
    if (audio_flac_struct) (void)drflac_close(audio_flac_struct);
    audio_flac_struct = NULL;

    // DO NOT call interrupt_disable() as this may clear an audio interrupt that has been fired
    // but not yet processed.

    UNLOCK_AUDIO("audio_stop");

    return kOk;
}

MmResult audio_term() {
    if (!audio_initialised) return kOk;
    MmResult result = audio_stop();
    if (mmb_options.audio) SDL_CloseAudio();
    audio_initialised = false;
    return result;
}

static float audio_callback_tone(int channel) {
    if (audio_tone_duration <= 0) {
        return 0.0f;
    } else {
        audio_tone_duration--;
        const int volume =
            (sine_table[(int)audio_phase_ac[channel][0]] - 2000) * mapping[TONE_VOLUME] / 2000;
        audio_phase_ac[channel][0] += audio_phase_m[channel][0];
        if (audio_phase_ac[channel][0] >= 4096.0) audio_phase_ac[channel][0] -= 4096.0;
        return (float)volume / 2000.0f;
    }
}

static float audio_callback_mod(int channel) {
    if (audio_play_buf->byte_count == 0) return 0.0f;

    float value = 0.0f;
    // Main MOD trac.
    {
        int16_t *buf = (int16_t *)audio_play_buf->data;

        if (audio_pos < audio_play_buf->byte_count) {
            value = (float)buf[audio_pos++] / 32768.0f * audio_filter_volume[channel];
        }

        if (audio_pos == audio_play_buf->byte_count) {
            audio_play_buf->byte_count = 0;
            SWAP(AudioBuffer *, audio_fill_buf, audio_play_buf);
            audio_pos = 0;
        }
    }

    float effect_value = 0.0f;
    // Effect WAV trac.
    if (audio_effect_state == P_WAV && audio_effect_play_buf->byte_count > 0) {
        float *buf = (float *)audio_effect_play_buf->data;

        if (audio_effect_pos < audio_effect_play_buf->byte_count) {
            effect_value = buf[audio_effect_pos++] * audio_filter_volume[channel];
            if (audio_effect_state == P_WAV && channel == LEFT_CHANNEL &&
                audio_effect_struct.channels == 1) {
                // Handle mono WAV playback.
                audio_effect_pos--;
            }
        }

        if (audio_effect_pos == audio_effect_play_buf->byte_count) {
            audio_effect_play_buf->byte_count = 0;
            SWAP(AudioBuffer *, audio_effect_fill_buf, audio_effect_play_buf);
            audio_effect_pos = 0;
        }
    }

    return value + effect_value;
}

// Callback for FLAC, MP3 and WAV files.
static float audio_callback_track(int channel) {
    if (audio_play_buf->byte_count == 0) return 0.0f;

    float value = 0.0f;
    float *buf = (float *)audio_play_buf->data;

    if (audio_pos < audio_play_buf->byte_count) {
        value = buf[audio_pos++] * audio_filter_volume[channel];
        if (channel == LEFT_CHANNEL) {
            // Handle mono playback.
            switch (audio_state) {
                case P_FLAC:
                    if (audio_flac_struct->channels == 1) audio_pos--;
                    break;
                case P_MP3:
                    if (audio_mp3_struct.channels == 1) audio_pos--;
                    break;
                case P_WAV:
                    if (audio_wav_struct.channels == 1) audio_pos--;
                    break;
                default:
                    break;
            }
        }
    }

    if (audio_pos == audio_play_buf->byte_count) {
        audio_play_buf->byte_count = 0;
        SWAP(AudioBuffer *, audio_fill_buf, audio_play_buf);
        audio_pos = 0;
    }

    return value;
}

static float audio_callback_sound(int channel) {
    static int noisedwell[2][MAXSOUNDS] = {0};
    static uint32_t noise[2][MAXSOUNDS] = {0};

    int volume = 0;
    int delta;

    for (int i = 0; i < MAXSOUNDS; ++i) {
        if (audio_sound[channel][i] == null_table) continue;

        if (audio_sound[channel][i] != white_noise_table) {
            delta = audio_sound[channel][i][(int)audio_phase_ac[channel][i]];
            audio_phase_ac[channel][i] += audio_phase_m[channel][i];
            if (audio_phase_ac[channel][i] >= 4096.0) audio_phase_ac[channel][i] -= 4096.0;
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

    return (float)volume / 2000.0f;
}

// For the moment 'len' is expected to always be 8 bytes (2 samples) for stereo.
static void audio_callback(void *userdata, Uint8 *stream, int len) {
    assert(len == 8);
    float *fstream = (float *)stream;
    for (int i = 0; i < 2; ++i) {
        switch (audio_state) {
            case P_TONE:
                fstream[i] = audio_callback_tone(i);
                break;
            case P_MOD:
                fstream[i] = audio_callback_mod(i);
                break;
            case P_MP3:
            case P_WAV:
            case P_FLAC:
                fstream[i] = audio_callback_track(i);
                break;
            case P_SOUND:
                fstream[i] = audio_callback_sound(i);
                break;
            default:
                fstream[i] = 0.0f;
                break;
        }
    }
}

static inline bool audio_is_last_track() {
    return audio_track_current == MAX_TRACKS - 1 ||
        !*audio_track_list[audio_track_current + 1];
}

MmResult audio_play_next() {
    MmResult result = kOk;

    if (!audio_initialised) {
        result = audio_init();
        if (FAILED(result)) return result;
    }

    LOCK_AUDIO("audio_play_next");

    switch (audio_state) {
        case P_FLAC:
        case P_MP3:
        case P_WAV:
            if (audio_is_last_track()) {
                console_puts("Last track is playing\r\n");
            } else {
                result = audio_play_next_track();
            }
            break;
        default:
            result = kAudioNothingToPlay;
    }

    UNLOCK_AUDIO("audio_play_next");
    return result;
}

MmResult audio_pause() {
    MmResult result = kOk;

    if (!audio_initialised) {
        result = audio_init();
        if (FAILED(result)) return result;
    }

    LOCK_AUDIO("audio_pause");

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

    UNLOCK_AUDIO("audio_pause");
    return result;
}

static inline bool audio_is_first_track() {
    return audio_track_current == 0;
}

MmResult audio_play_previous() {
    MmResult result = kOk;

    if (!audio_initialised) {
        result = audio_init();
        if (FAILED(result)) return result;
    }

    LOCK_AUDIO("audio_play_previous");

    switch (audio_state) {
        case P_FLAC:
        case P_MOD:
        case P_MP3:
        case P_WAV:
            if (audio_is_first_track()) {
                console_puts("First track is playing\r\n");
            } else {
                audio_track_current -= 2;
                result = audio_play_next_track();
            }
            break;
        default:
            result = kAudioNothingToPlay;
    }

    UNLOCK_AUDIO("audio_play_previous");
    return result;
}

static bool audio_is_valid_sample_rate(unsigned sample_rate) {
    return sample_rate == 8000 || sample_rate == 16000 || sample_rate == 22050 ||
           sample_rate == 44100 || sample_rate == 48000;
}

/**
 * Opens an audio file.
 *
 * @param  filename  Name of the file including extension.
 */
static MmResult audio_open_file(const char *filename) {
    MmResult result = audio_close_file();
    if (SUCCEEDED(result)) {
        audio_fnbr = file_find_free();
        result = file_open(filename, "rb", audio_fnbr);
    }
    return result;
}

static MmResult audio_stop_effect() {
    if (audio_effect_state == P_WAV) {
        (void)drwav_uninit(&audio_effect_struct);
        audio_effect_free_buffers();
        audio_effect_state = P_NOTHING;
        audio_close_file();
    }
    return kOk;
}

static MmResult audio_play_effect_internal(const char *filename) {
    char _filename[STRINGSIZE];
    MmResult result = path_try_extension(filename, ".WAV", _filename, STRINGSIZE);

    if (SUCCEEDED(result)) {
        result = audio_open_file(_filename);
    }

    if (SUCCEEDED(result)) {
        drwav_allocation_callbacks allocationCallbacks;
        allocationCallbacks.pUserData = &audio_dummy_data;
        allocationCallbacks.onMalloc = audio_malloc;
        allocationCallbacks.onRealloc = audio_realloc;
        allocationCallbacks.onFree = audio_free_memory;

        if (drwav_init(&audio_effect_struct, (drwav_read_proc)audio_on_read,
                       (drwav_seek_proc)audio_on_seek, NULL, &allocationCallbacks) == DRWAV_FALSE)
            result = kAudioWavInitialisationFailed;
    }

    if (SUCCEEDED(result) && audio_effect_struct.sampleRate != audio_mod_sample_rate) {
        result = kAudioSampleRateMismatch;
    }

    if (SUCCEEDED(result)) {
        result = audio_effect_alloc_buffers(WAV_BUFFER_SIZE * 4);
    }

    if (SUCCEEDED(result)) {
        audio_effect_buf[0].byte_count =
            drwav_read_pcm_frames_f32(&audio_effect_struct, WAV_BUFFER_SIZE / 2,
                                      (float *)audio_effect_buf[0].data) *
            audio_effect_struct.channels;
        audio_effect_pos = 0;
        audio_effect_state = P_WAV;
    }

    return result;
}

MmResult audio_play_effect(const char *filename, const char *interrupt) {
    MmResult result = kOk;

    if (!audio_initialised) {
        result = audio_init();
        if (FAILED(result)) return result;
    }

    LOCK_AUDIO("audio_play_effect");

    if (audio_state != P_MOD) result = kAudioNoModFile;
    if (SUCCEEDED(result)) result = audio_stop_effect();
    if (SUCCEEDED(result)) result = audio_play_effect_internal(filename);
    if (SUCCEEDED(result)) interrupt_enable(kInterruptAudio2, interrupt);

    UNLOCK_AUDIO("audio_play_effect");
    return result;
}

static MmResult audio_play_modfile_internal(const char *filename) {
    MmResult result = audio_open_file(filename);

    // Read the file into 'audio_mod_buf'
    // TODO: If file_lof() or file_read() report error this will leave audio device paused.
    int size = 0;
    if (SUCCEEDED(result)) {
        size = file_lof(audio_fnbr);
        audio_alloc_mod_buf(size);
        file_read(audio_fnbr, audio_mod_buf, size);
        result = audio_close_file();
    }

    if (SUCCEEDED(result)) {
        audio_alloc_buffers(WAV_BUFFER_SIZE);
        hxcmod_init(&audio_mod_context);
        hxcmod_setcfg(&audio_mod_context, audio_mod_sample_rate, 1, 1);
        hxcmod_load(&audio_mod_context, (void *)audio_mod_buf, size);
        (void)hxcmod_fillbuffer(&audio_mod_context, (msample *)audio_buf[0].data,
                                WAV_BUFFER_SIZE / 4, NULL, audio_mod_loop ? 0 : 1);
        audio_buf[0].byte_count = WAV_BUFFER_SIZE / 2;
        result = audio_configure(audio_mod_sample_rate, 2);
    }

    if (SUCCEEDED(result)) {
        audio_state = P_MOD;
    }

    return result;
}

static MmResult audio_play_flac_internal(const char *filename) {
    MmResult result = audio_open_file(filename);

    if (SUCCEEDED(result)) {
        drflac_allocation_callbacks allocationCallbacks;
        allocationCallbacks.pUserData = &audio_dummy_data;
        allocationCallbacks.onMalloc = audio_malloc;
        allocationCallbacks.onRealloc = audio_realloc;
        allocationCallbacks.onFree = audio_free_memory;

        if (audio_flac_struct) (void)drflac_close(audio_flac_struct);
        audio_flac_struct =
            drflac_open((drflac_read_proc)audio_on_read, (drflac_seek_proc)audio_on_seek,
                        &audio_dummy_data, &allocationCallbacks);
        if (!audio_flac_struct) result = kAudioFlacInitialisationFailed;
    }

    if (SUCCEEDED(result)) {
        result = audio_alloc_buffers(WAV_BUFFER_SIZE * 4);
    }

    if (SUCCEEDED(result)) {
        audio_buf[0].byte_count = drflac_read_pcm_frames_f32(audio_flac_struct, WAV_BUFFER_SIZE / 2,
                                                             (float *)audio_buf[0].data) *
                                  audio_flac_struct->channels;
        result = audio_configure(audio_flac_struct->sampleRate, 2);
    }

    if (SUCCEEDED(result)) {
        audio_state = P_FLAC;
    }

    return result;
}

static MmResult audio_play_mp3_internal(const char *filename) {
    MmResult result = audio_open_file(filename);

    if (SUCCEEDED(result)) {
        drmp3_allocation_callbacks allocationCallbacks;
        allocationCallbacks.pUserData = &audio_dummy_data;
        allocationCallbacks.onMalloc = audio_malloc;
        allocationCallbacks.onRealloc = audio_realloc;
        allocationCallbacks.onFree = audio_free_memory;

        if (drmp3_init(&audio_mp3_struct, (drmp3_read_proc)audio_on_read,
                       (drmp3_seek_proc)audio_on_seek, NULL, &allocationCallbacks) == DRMP3_FALSE) {
            result = kAudioMp3InitialisationFailed;
        }
    }

    if (SUCCEEDED(result)) {
        result = audio_alloc_buffers(WAV_BUFFER_SIZE * 4);
    }

    if (SUCCEEDED(result)) {
        audio_buf[0].byte_count = drmp3_read_pcm_frames_f32(&audio_mp3_struct, WAV_BUFFER_SIZE / 2,
                                                            (float *)audio_buf[0].data) *
                                  audio_mp3_struct.channels;
        result = audio_configure(audio_mp3_struct.sampleRate, 2);
    }

    if (SUCCEEDED(result)) {
        audio_state = P_MP3;
    }

    return result;
}

static MmResult audio_play_wav_internal(const char *filename) {
    MmResult result = audio_open_file(filename);

    if (SUCCEEDED(result)) {
        drwav_allocation_callbacks allocationCallbacks;
        allocationCallbacks.pUserData = &audio_dummy_data;
        allocationCallbacks.onMalloc = audio_malloc;
        allocationCallbacks.onRealloc = audio_realloc;
        allocationCallbacks.onFree = audio_free_memory;

        if (drwav_init(&audio_wav_struct, (drwav_read_proc)audio_on_read,
                       (drwav_seek_proc)audio_on_seek, NULL, &allocationCallbacks) == DRWAV_FALSE)
            result = kAudioWavInitialisationFailed;
    }

    if (SUCCEEDED(result)) {
        result = audio_alloc_buffers(WAV_BUFFER_SIZE * 4);
    }

    if (SUCCEEDED(result)) {
        audio_buf[0].byte_count = drwav_read_pcm_frames_f32(&audio_wav_struct, WAV_BUFFER_SIZE / 2,
                                                            (float *)audio_buf[0].data) *
                                  audio_wav_struct.channels;
        result = audio_configure(audio_wav_struct.sampleRate, 2);
    }

    if (SUCCEEDED(result)) {
        audio_state = P_WAV;
    }

    return result;
}

extern const char *CurrentLinePtr; // MMBasic.c

static MmResult audio_play_next_track() {
    audio_track_current++;
    const char *next_track = audio_track_list[audio_track_current];
    if (!*next_track) return kAudioNoMoreTracks;
    if (!CurrentLinePtr) {
        console_puts("Now playing: ");
        console_puts(next_track);
        console_puts("\r\n");
    }
    MmResult result = kOk;
    if (path_has_extension(next_track, ".FLAC", true)) {
        result = audio_play_flac_internal(next_track);
    } else if (path_has_extension(next_track, ".MOD", true)) {
        result = audio_play_modfile_internal(next_track);
    } else if (path_has_extension(next_track, ".MP3", true)) {
        result = audio_play_mp3_internal(next_track);
    } else if (path_has_extension(next_track, ".WAV", true)) {
        result = audio_play_wav_internal(next_track);
    } else {
        result = kInternalFault;
    }
    return result;
}

MmResult audio_play_modfile(const char *filename, unsigned sample_rate, const char *interrupt) {
    MmResult result = kOk;

    if (!audio_initialised) {
        result = audio_init();
        if (FAILED(result)) return result;
    }

    LOCK_AUDIO("audio_play_modfile");

    if (!audio_is_valid_sample_rate(sample_rate)) result = kAudioInvalidSampleRate;
    if (SUCCEEDED(result) && audio_state != P_NOTHING) result = kAudioInUse;
    if (SUCCEEDED(result)) result = audio_fill_track_list(filename, ".MOD");
    if (SUCCEEDED(result)) {
        audio_mod_loop = (interrupt == NULL);
        audio_mod_sample_rate = sample_rate;
        result = audio_play_next_track();
    }
    if (SUCCEEDED(result)) interrupt_enable(kInterruptAudio1, interrupt);

    UNLOCK_AUDIO("audio_play_modfile");
    return result;
}

MmResult audio_play_modsample(uint8_t sample_num, uint8_t channel_num, uint8_t volume,
                              unsigned sample_rate) {
    MmResult result = kOk;

    if (!audio_initialised) {
        result = audio_init();
        if (FAILED(result)) return result;
    }

    if (!audio_is_valid_sample_rate(sample_rate)) {
        result = kAudioInvalidSampleRate;
    }

    LOCK_AUDIO("audio_play_modsample");

    if (SUCCEEDED(result) && audio_state != P_MOD) {
        result = kAudioNoModFile;
    }

    if (SUCCEEDED(result)) {
        hxcmod_playsoundeffect(&audio_mod_context, sample_num - 1, channel_num - 1,
                               max(0, volume - 1), 3579545 / sample_rate);
    }

    UNLOCK_AUDIO("audio_play_modsample");
    return result;
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

    LOCK_AUDIO("audio_play_sound");

    if (audio_state != P_NOTHING && audio_state != P_SOUND && audio_state != P_PAUSE_SOUND) {
        result = kAudioInUse;
    }

    if (SUCCEEDED(result) && !audio_is_valid_frequency(frequency)) {
        result = kAudioInvalidFrequency;
    }

    const uint16_t *previous[2] = {audio_sound[LEFT_CHANNEL][sound_no],
                                   audio_sound[RIGHT_CHANNEL][sound_no]};

    audio_sound[LEFT_CHANNEL][sound_no] = null_table;
    audio_sound[RIGHT_CHANNEL][sound_no] = null_table;

    for (int c = LEFT_CHANNEL /* 0 */; c <= RIGHT_CHANNEL /* 1 */; ++c) {
        if (c == LEFT_CHANNEL && !(channel & kChannelLeft)) {
            continue;
        } else if (c == RIGHT_CHANNEL && !(channel & kChannelRight)) {
            continue;
        }
        switch (type) {
            case kSoundTypeSine:
                audio_sound[c][sound_no] = sine_table;
                break;
            case kSoundTypeSquare:
                audio_sound[c][sound_no] = square_table;
                break;
            case kSoundTypeTriangular:
                audio_sound[c][sound_no] = triangular_table;
                break;
            case kSoundTypeSawTooth:
                audio_sound[c][sound_no] = saw_tooth_table;
                break;
            case kSoundTypePeriodicNoise:
                audio_sound[c][sound_no] = periodic_noise_table;
                break;
            case kSoundTypeWhiteNoise:
                audio_sound[c][sound_no] = white_noise_table;
                break;
            case kSoundTypeNull:
                // Already set to null_table.
                break;
            default:
                result = kInternalFault;
        }
    }

    if (SUCCEEDED(result)) {
        for (int c = LEFT_CHANNEL /* 0 */; c <= RIGHT_CHANNEL /* 1 */; ++c) {
            if (c == LEFT_CHANNEL && !(channel & kChannelLeft)) {
                continue;
            } else if (c == RIGHT_CHANNEL && !(channel & kChannelRight)) {
                continue;
            }

            if (previous[c] == null_table) audio_phase_ac[c][sound_no] = 0.0f;
            audio_phase_m[c][sound_no] = audio_sound[c][sound_no] == white_noise_table
                                             ? frequency
                                             : frequency / (float)AUDIO_SAMPLE_RATE * 4096.0f;
            audio_sound_volume[c][sound_no] = (volume * 41) / 25;
        }
        result = audio_configure(AUDIO_SAMPLE_RATE, 2);
    }

    if (SUCCEEDED(result)) {
        audio_state = P_SOUND;
    }

    UNLOCK_AUDIO("audio_play_sound");
    return result;
}

MmResult audio_play_tone(float f_left, float f_right, int64_t duration, const char *interrupt) {
    MmResult result = kOk;

    if (!audio_initialised) {
        result = audio_init();
        if (FAILED(result)) return result;
    }

    LOCK_AUDIO("audio_play_tone");

    if (audio_state != P_NOTHING && audio_state != P_TONE && audio_state != P_PAUSE_TONE) {
        result = kAudioInUse;
    }

    if (SUCCEEDED(result) &&
        !(audio_is_valid_frequency(f_left) && audio_is_valid_frequency(f_right))) {
        result = kAudioInvalidFrequency;
    }

    if (SUCCEEDED(result) && duration == 0) {
        UNLOCK_AUDIO("audio_play_tone");
        return kOk;
    }

    if (SUCCEEDED(result)) {
        uint64_t play_duration = duration == -1
                ? UINT64_MAX
                : AUDIO_SAMPLE_RATE * (uint64_t) duration / 1000;

        if (play_duration != UINT64_MAX && f_left >= 10.0) {
            float hw = ((float)AUDIO_SAMPLE_RATE / f_left);
            int x = (int)((float)(play_duration) / hw) + 1;
            play_duration = (uint64_t)((float)x * hw);
        }
        audio_phase_m[0][LEFT_CHANNEL] = f_left / (float)AUDIO_SAMPLE_RATE * 4096.0f;
        audio_phase_m[0][RIGHT_CHANNEL] = f_right / (float)AUDIO_SAMPLE_RATE * 4096.0f;

        audio_tone_duration = play_duration;

        result = audio_configure(AUDIO_SAMPLE_RATE, 2);
    }

    if (SUCCEEDED(result)) {
        audio_state = P_TONE;
        interrupt_enable(kInterruptAudio1, interrupt);
    }

    UNLOCK_AUDIO("audio_play_tone");
    return result;
}

MmResult audio_resume() {
    MmResult result = kOk;

    if (!audio_initialised) {
        result = audio_init();
        if (FAILED(result)) return result;
    }

    LOCK_AUDIO("audio_resume");

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

    UNLOCK_AUDIO("audio_resume");
    return result;
}

static MmResult audio_play_file(const char *filename, const char *extension,
                                const char *interrupt) {
    MmResult result = kOk;

    if (!audio_initialised) {
        result = audio_init();
        if (FAILED(result)) return result;
    }

    LOCK_AUDIO("audio_play_file");

    if (audio_state != P_NOTHING) result = kAudioInUse;
    if (SUCCEEDED(result)) result = audio_fill_track_list(filename, extension);
    if (SUCCEEDED(result)) result = audio_play_next_track();
    if (SUCCEEDED(result)) interrupt_enable(kInterruptAudio1, interrupt);

    UNLOCK_AUDIO("audio_play_file");
    return result;
}

MmResult audio_play_flac(const char *filename, const char *interrupt) {
    return audio_play_file(filename, ".FLAC", interrupt);
}

MmResult audio_play_mp3(const char *filename, const char *interrupt) {
    return audio_play_file(filename, ".MP3", interrupt);
}

MmResult audio_play_wav(const char *filename, const char *interrupt) {
    return audio_play_file(filename, ".WAV", interrupt);
}

MmResult audio_background_tasks() {
    if (!audio_initialised) return kOk;

    SDL_LockAudio();

    switch (audio_state) {
        case P_FLAC:
        case P_MOD:
        case P_MP3:
        case P_WAV:
            break;
        case P_TONE:
            if (audio_tone_duration <= 0) {
                interrupt_fire(kInterruptAudio1);
                (void) audio_stop();
            }
            SDL_UnlockAudio();
            return kOk;
        default:
            // Nothing to do.
            SDL_UnlockAudio();
            return kOk;
    }

    // Fill the main trac buffer empty.
    if (audio_fill_buf->byte_count == 0) {
        char *buf = audio_fill_buf->data;
        switch (audio_state) {
            case P_FLAC: {
                audio_fill_buf->byte_count =
                    drflac_read_pcm_frames_f32(audio_flac_struct, WAV_BUFFER_SIZE / 2,
                                               (float *)buf) *
                    audio_flac_struct->channels;
                break;
            }

            case P_MOD: {
                if (hxcmod_fillbuffer(&audio_mod_context, (msample *)buf, WAV_BUFFER_SIZE / 4, NULL,
                                      audio_mod_loop ? 0 : 1)) {
                    // Stop the music!
                    audio_fill_buf->byte_count = 0;
                    audio_play_buf->byte_count = 0;
                } else {
                    audio_fill_buf->byte_count = WAV_BUFFER_SIZE / 2;
                }
                break;
            }

            case P_WAV: {
                audio_fill_buf->byte_count =
                    drwav_read_pcm_frames_f32(&audio_wav_struct, WAV_BUFFER_SIZE / 2,
                                              (float *)buf) *
                    audio_wav_struct.channels;
                break;
            }

            case P_MP3: {
                audio_fill_buf->byte_count =
                    drmp3_read_pcm_frames_f32(&audio_mp3_struct, WAV_BUFFER_SIZE / 2,
                                              (float *)buf) *
                    audio_mp3_struct.channels;
                break;
            }

            default:
                break;
        }
    }

    // Fill the effect buffer if empty.
    if (audio_effect_state == P_WAV && audio_effect_fill_buf->byte_count == 0) {
        char *buf = audio_effect_fill_buf->data;
        switch (audio_effect_state) {
            case P_WAV: {
                audio_effect_fill_buf->byte_count =
                    drwav_read_pcm_frames_f32(&audio_effect_struct, WAV_BUFFER_SIZE / 2,
                                              (float *)buf) *
                    audio_effect_struct.channels;
                break;
            }

            default:
                break;
        }
    }

    // Check if FLAC, MOD, MP3 or WAV playback has finished.
    if (audio_fill_buf->byte_count == 0 && audio_play_buf->byte_count == 0) {
        switch (audio_state) {
            case P_FLAC:
                (void)drflac_close(audio_flac_struct);
                audio_flac_struct = NULL;
                break;
            case P_MP3:
                (void)drmp3_uninit(&audio_mp3_struct);
                break;
            case P_WAV:
                (void)drwav_uninit(&audio_wav_struct);
                break;
            default:
                break;
        }

        MmResult result = audio_play_next_track();
        if (FAILED(result)) {
            audio_stop();
            interrupt_fire(kInterruptAudio1);
        }
    }

    // Check if WAV effect playback has finished.
    if (audio_effect_state == P_WAV && audio_effect_fill_buf->byte_count == 0 &&
        audio_effect_play_buf->byte_count == 0) {
        (void)audio_stop_effect();
        interrupt_fire(kInterruptAudio2);
    }

    SDL_UnlockAudio();

    return kOk;
}

MmResult audio_set_volume(uint8_t left, uint8_t right) {
    audio_filter_volume[LEFT_CHANNEL] = (float)mapping[left] / 2001.0f;
    audio_filter_volume[RIGHT_CHANNEL] = (float)mapping[right] / 2001.0f;
    return kOk;
}
