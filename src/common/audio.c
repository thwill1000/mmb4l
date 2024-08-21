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

#include <SDL.h>
#include <assert.h>
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
#include "cstring.h"
#include "error.h"
#include "events.h"
#include "file.h"
#include "interrupt.h"
#include "memory.h"
#include "mmresult.h"
#include "path.h"
#include "utility.h"

#define BUFFER_SIZE 1
#define AUDIO_SAMPLE_RATE 44100UL
#define WAV_BUFFER_SIZE 16384
#define LEFT_CHANNEL 0
#define RIGHT_CHANNEL 1
#define TONE_VOLUME 100

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

/** Configures the SDL Audio sample rate and number of channels. */
static MmResult audio_configure(int sample_rate, int num_channels) {
    // printf("audio_configure()\n");
    // printf("  sample_rate = %d\n", sample_rate);
    // printf("  num_channels = %d\n", num_channels);

    if (audio_initialised && audio_current_spec.freq == sample_rate &&
        audio_current_spec.channels == num_channels)
        return kOk;

    SDL_CloseAudio();
    audio_current_spec = (SDL_AudioSpec){
        .format = AUDIO_F32,
        .channels = num_channels,
        .freq = sample_rate,
        .samples = BUFFER_SIZE,
        .callback = audio_callback,
    };

    if (SUCCEEDED(SDL_OpenAudio(&audio_current_spec, NULL))) {
        return kOk;
    } else {
        return kAudioApiError;
    }
}

MmResult audio_init() {
    if (audio_initialised) return kOk;
    MmResult result = events_init();

    if (SUCCEEDED(result)) {
        audio_tables_init();
        audio_initialised = true;
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

static MmResult audio_close_file() {
    if (audio_fnbr != -1) {
        MmResult result = file_close(audio_fnbr);
        audio_fnbr = -1;
        return result;
    } else {
        return kOk;
    }
}

MmResult audio_term() {
    if (!audio_initialised) return kOk;

    SDL_LockAudioDevice(1);

    (void)audio_close_file();
    audio_state = P_NOTHING;
    audio_effect_state = P_NOTHING;
    audio_free_buffers();
    audio_effect_free_buffers();
    audio_tone_duration = 0;
    for (int snd = 0; snd < MAXSOUNDS; ++snd) {
        for (int channel = LEFT_CHANNEL; channel <= RIGHT_CHANNEL; ++channel) {
            audio_phase_m[channel][snd] = 0.0f;
            audio_phase_ac[channel][snd] = 0.0f;
            audio_sound[channel][snd] = null_table;
        }
    }

    // DO NOT call interrupt_disable() as this may clear an audio interrupt that has been fired
    // but not yet processed.

    SDL_UnlockAudioDevice(1);

    return kOk;
}

static float audio_callback_tone(int channel) {
    if (audio_tone_duration <= 0) {
        interrupt_fire(kInterruptAudio1);
        audio_term();
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
        if (audio_state == P_WAV && channel == LEFT_CHANNEL && audio_wav_struct.channels == 1) {
            // Handle mono WAV playback.
            audio_pos--;
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
    for (int i = 0; i < BUFFER_SIZE; i += 2) {  // The 2 is irrelevant, BUFFER_SIZE = 1.
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
                fstream[i] = audio_callback_track(0);
                fstream[i + 1] = audio_callback_track(1);
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
    return sample_rate == 8000 || sample_rate == 16000 || sample_rate == 22050 ||
           sample_rate == 44100 || sample_rate == 48000;
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

static MmResult audio_play_effect_internal(const char *filename) {
    MmResult result = audio_open_file(filename, ".WAV");

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

    SDL_PauseAudio(1);

    if (audio_state != P_MOD) result = kAudioNoModFile;
    if (SUCCEEDED(result)) result = audio_play_effect_internal(filename);
    if (SUCCEEDED(result)) interrupt_enable(kInterruptAudio2, interrupt);

    SDL_PauseAudio(0);
    return result;
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

    // Read the file into 'audio_mod_buf'
    // TODO: If file_lof() or file_read() report error this will leave audio device paused.
    int size = 0;
    if (SUCCEEDED(result)) {
        size = file_lof(audio_fnbr);
        audio_mod_buf = (char *)GetMemory(size + 256);
        file_read(audio_fnbr, audio_mod_buf, size);
        result = audio_close_file();
    }

    if (SUCCEEDED(result)) {
        audio_alloc_buffers(WAV_BUFFER_SIZE);

        audio_mod_loop = (interrupt == NULL);
        audio_mod_sample_rate = sample_rate;

        hxcmod_init(&audio_mod_context);
        hxcmod_setcfg(&audio_mod_context, sample_rate, 1, 1);
        hxcmod_load(&audio_mod_context, (void *)audio_mod_buf, size);
        (void)hxcmod_fillbuffer(&audio_mod_context, (msample *)audio_buf[0].data,
                                WAV_BUFFER_SIZE / 4, NULL, audio_mod_loop ? 0 : 1);
        audio_buf[0].byte_count = WAV_BUFFER_SIZE / 2;
        audio_state = P_MOD;
        interrupt_enable(kInterruptAudio1, interrupt);
    }

    SDL_PauseAudio(0);
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

    SDL_LockAudioDevice(1);

    if (SUCCEEDED(result) && audio_state != P_MOD) {
        result = kAudioNoModFile;
    }

    if (SUCCEEDED(result)) {
        hxcmod_playsoundeffect(&audio_mod_context, sample_num - 1, channel_num - 1,
                               max(0, volume - 1), 3579545 / sample_rate);
    }

    SDL_UnlockAudioDevice(1);

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

    SDL_PauseAudio(1);

    if (audio_state != P_NOTHING && audio_state != P_SOUND && audio_state != P_PAUSE_SOUND) {
        result = kSoundInUse;
    }

    if (SUCCEEDED(result) && !audio_is_valid_frequency(frequency)) {
        result = kSoundInvalidFrequency;
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
            audio_phase_m[c][sound_no] = audio_sound[c][sound_no] == white_noise_table
                                             ? frequency
                                             : frequency / (float)AUDIO_SAMPLE_RATE * 4096.0f;
            audio_sound_volume[c][sound_no] = (volume * 41) / 25;
        }

        audio_state = P_SOUND;
    }

    SDL_PauseAudio(0);
    return result;
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

    if (SUCCEEDED(result) &&
        !(audio_is_valid_frequency(f_left) && audio_is_valid_frequency(f_right))) {
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
        uint64_t play_duration =
            duration == -1 ? 0xFFFFFFFFFFFFFFFF : AUDIO_SAMPLE_RATE * duration / 1000;

        if (play_duration != 0xffffffffffffffff && f_left >= 10.0) {
            float hw = ((float)AUDIO_SAMPLE_RATE / f_left);
            int x = (int)((float)(play_duration) / hw) + 1;
            play_duration = (uint64_t)((float)x * hw);
        }
        audio_phase_m[0][LEFT_CHANNEL] = f_left / (float)AUDIO_SAMPLE_RATE * 4096.0f;
        audio_phase_m[0][RIGHT_CHANNEL] = f_right / (float)AUDIO_SAMPLE_RATE * 4096.0f;

        audio_tone_duration = play_duration;
        audio_state = P_TONE;

        interrupt_enable(kInterruptAudio1, interrupt);
    }

    SDL_PauseAudio(0);
    return result;
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

        audio_flac_struct =
            drflac_open((drflac_read_proc)audio_on_read, (drflac_seek_proc)audio_on_seek,
                        &audio_dummy_data, &allocationCallbacks);
        if (!audio_flac_struct) result = kAudioFlacInitialisationFailed;
    }

    if (SUCCEEDED(result)) {
        result = audio_alloc_buffers(WAV_BUFFER_SIZE * 4);
    }

    if (SUCCEEDED(result)) {
        result = audio_configure(audio_flac_struct->sampleRate, 2);
    }

    if (SUCCEEDED(result)) {
        audio_buf[0].byte_count = drflac_read_pcm_frames_f32(audio_flac_struct, WAV_BUFFER_SIZE / 2,
                                                             (float *)audio_buf[0].data) *
                                  audio_flac_struct->channels;
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
    if (SUCCEEDED(result)) interrupt_enable(kInterruptAudio1, interrupt);

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

        if (drmp3_init(&audio_mp3_struct, (drmp3_read_proc)audio_on_read,
                       (drmp3_seek_proc)audio_on_seek, NULL, &allocationCallbacks) == DRMP3_FALSE) {
            result = kAudioMp3InitialisationFailed;
        }
    }

    if (SUCCEEDED(result)) {
        result = audio_alloc_buffers(WAV_BUFFER_SIZE * 4);
    }

    if (SUCCEEDED(result)) {
        result = audio_configure(audio_mp3_struct.sampleRate, 2);
    }

    if (SUCCEEDED(result)) {
        audio_buf[0].byte_count = drmp3_read_pcm_frames_f32(&audio_mp3_struct, WAV_BUFFER_SIZE / 2,
                                                            (float *)audio_buf[0].data) *
                                  audio_mp3_struct.channels;
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
    if (SUCCEEDED(result)) interrupt_enable(kInterruptAudio1, interrupt);

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

        if (drwav_init(&audio_wav_struct, (drwav_read_proc)audio_on_read,
                       (drwav_seek_proc)audio_on_seek, NULL, &allocationCallbacks) == DRWAV_FALSE)
            result = kAudioWavInitialisationFailed;
    }

    if (SUCCEEDED(result)) {
        result = audio_alloc_buffers(WAV_BUFFER_SIZE * 4);
    }

    if (SUCCEEDED(result)) {
        result = audio_configure(audio_wav_struct.sampleRate, 2 /*audio_wav_struct.channels*/);
    }

    if (SUCCEEDED(result)) {
        audio_buf[0].byte_count = drwav_read_pcm_frames_f32(&audio_wav_struct, WAV_BUFFER_SIZE / 2,
                                                            (float *)audio_buf[0].data) *
                                  audio_wav_struct.channels;
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
    if (SUCCEEDED(result)) interrupt_enable(kInterruptAudio1, interrupt);

    SDL_PauseAudio(0);
    return result;
}

MmResult audio_background_tasks() {
    SDL_LockAudioDevice(1);

    switch (audio_state) {
        case P_FLAC:
        case P_MOD:
        case P_MP3:
        case P_WAV:
            break;
        default:
            // Nothing to do.
            SDL_UnlockAudioDevice(1);
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

        audio_term();
        interrupt_fire(kInterruptAudio1);
    }

    // Check if WAV effect playback has finished.
    if (audio_effect_state == P_WAV && audio_effect_fill_buf->byte_count == 0 &&
        audio_effect_play_buf->byte_count == 0) {
        (void)drwav_uninit(&audio_effect_struct);
        audio_effect_free_buffers();
        audio_effect_state = P_NOTHING;
        audio_close_file();
        interrupt_fire(kInterruptAudio2);
    }

    SDL_UnlockAudioDevice(1);

    return kOk;
}

MmResult audio_set_volume(uint8_t left, uint8_t right) {
    audio_filter_volume[LEFT_CHANNEL] = (float)mapping[left] / 2001.0f;
    audio_filter_volume[RIGHT_CHANNEL] = (float)mapping[right] / 2001.0f;
    return kOk;
}
