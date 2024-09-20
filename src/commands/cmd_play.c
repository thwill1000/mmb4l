/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_play.c

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

#include "../common/audio.h"
#include "../common/audio_tables.h"
#include "../common/error.h"
#include "../common/mmb4l.h"
#include "../common/utility.h"

#include <limits.h>
#include <stdint.h>
#include <strings.h>

static MmResult cmd_play_continue(const char *p) {
    ERROR_UNIMPLEMENTED("PLAY CONTINUE");
    return kUnimplemented;
}

/** PLAY EFFECT file$ [, interrupt] */
static MmResult cmd_play_effect(const char *p) {
    getargs(&p, 3, ",");
    if (argc != 1 && argc != 3) return kArgumentCount;

    const char *file = getCstring(argv[0]);
    const char *interrupt = (argc > 1) ? GetIntAddress(argv[2]) : NULL;

    return audio_play_effect(file, interrupt);
}

/** PLAY FLAC file$ [, interrupt] */
static MmResult cmd_play_flac(const char *p) {
    getargs(&p, 3, ",");
    if (argc != 1 && argc != 3) return kArgumentCount;

    const char *file = getCstring(argv[0]);
    const char *interrupt = (argc > 1) ? GetIntAddress(argv[2]) : NULL;

    return audio_play_flac(file, interrupt);
}

static MmResult cmd_play_halt(const char *p) {
    ERROR_UNIMPLEMENTED("PLAY HALT");
    return kUnimplemented;
}

static MmResult cmd_play_load_sound(const char *p) {
    ERROR_UNIMPLEMENTED("PLAY LOAD SOUND");
    return kUnimplemented;
}

static MmResult cmd_play_midi(const char *p) {
    ERROR_UNIMPLEMENTED("PLAY MIDI");
    return kUnimplemented;
}

static MmResult cmd_play_midifile(const char *p) {
    ERROR_UNIMPLEMENTED("PLAY MIDIFILE");
    return kUnimplemented;
}

static MmResult cmd_play_next(const char *p) {
    skipspace(p);
    if (!parse_is_end(p)) error_throw(kUnexpectedText);
    return audio_play_next();
}

static MmResult cmd_play_note(const char *p) {
    ERROR_UNIMPLEMENTED("PLAY NOTE");
    return kUnimplemented;
}

/**
 * PLAY MODFILE file$ [, sample_rate] [, interrupt]
 *
 * PicoMite{VGA}: 'sample_rate' argument is unsupported.
 * CMM2/MMB4W:    'interrupt' argument is unsupported.
 */
static MmResult cmd_play_modfile(const char *p) {
    getargs(&p, 5, ",");
    if (argc != 1 && argc != 3 && argc != 5) return kArgumentCount;

    const char *filename = getCstring(argv[0]);
    unsigned sample_rate = 44100;
    const char *interrupt = NULL;
    switch (mmb_options.simulate) {
        case kSimulateMmb4l:
            sample_rate = (argc > 1) ? getint(argv[2], 0, 48000) : 44100;
            interrupt = (argc > 3) ? GetIntAddress(argv[4]) : NULL;
            break;
        case kSimulateGameMite:
        case kSimulatePicoMiteVga:
            if (argc == 5) return kUnsupportedParameterOnCurrentDevice;
            interrupt = (argc > 1) ? GetIntAddress(argv[2]) : NULL;
            break;
        case kSimulateCmm2:
        case kSimulateMmb4w:
            if (argc == 5) return kUnsupportedParameterOnCurrentDevice;
            sample_rate = (argc > 1) ? getint(argv[2], 0, 48000) : 44100;
            break;
        default:
            return kInternalFault;
    }

    return audio_play_modfile(filename, sample_rate, interrupt);
}

/** PLAY MODSAMPLE sample_num, channel_num [, volume] [, sample_rate] */
static MmResult cmd_play_modsample(const char *p) {
    getargs(&p, 7, ",");
    if (argc != 3 && argc != 5 && argc != 7) return kArgumentCount;
    const uint8_t sample_num = (uint8_t) getint(argv[0], 1, 32);
    const uint8_t channel_num = (uint8_t) getint(argv[2], 1, 4);
    const uint8_t volume = max(0, (argc >= 5) ? (uint8_t) getint(argv[4], 0, 64) : 64);
    const unsigned sample_rate = (argc == 7) ? (uint32_t) getint(argv[6], 150, 500000) : 16000;

    return audio_play_modsample(sample_num, channel_num, volume, sample_rate);
}

/** PLAY PAUSE */
static MmResult cmd_play_pause(const char *p) {
    skipspace(p);
    if (!parse_is_end(p)) return kUnexpectedText;
    return audio_pause();
}

/** PLAY PREVIOUS */
static MmResult cmd_play_previous(const char *p) {
    skipspace(p);
    if (!parse_is_end(p)) return kUnexpectedText;
    return audio_play_previous();
}

/** PLAY RESUME */
static MmResult cmd_play_resume(const char *p) {
    skipspace(p);
    if (!parse_is_end(p)) return kUnexpectedText;
    return audio_resume();
}

/** PLAY SOUND sound_no, channel_no, type [, frequency] [, volume] */
static MmResult cmd_play_sound(const char *p) {
    getargs(&p, 9, ",");
    if (argc != 5 && argc != 7 && argc != 9) return kArgumentCount;

    int sound_no = getint(argv[0], 1, MAXSOUNDS) - 1;

    Channel channel = kChannelBoth;
    if (checkstring(argv[2], "L")) {
        channel = kChannelLeft;
    } else if (checkstring(argv[2], "R")) {
        channel = kChannelRight;
    } else if (checkstring(argv[2], "B")) {
        channel = kChannelBoth;
    } else {
        char *s = getCstring(argv[2]);
        if (strcasecmp("L", s) == 0) {
            channel = kChannelLeft;
        } else if (strcasecmp("R", s) == 0) {
            channel = kChannelRight;
        } else if (strcasecmp("B", s) == 0) {
            channel = kChannelBoth;
        } else {
            MMRESULT_RETURN_EX(kSyntax, "Channel number must be L, R or B");
        }
        ClearSpecificTempMemory(s);
    }

    SoundType type = kSoundTypeNull;
    if (checkstring(argv[4], "N")) {
        type = kSoundTypeWhiteNoise;
    } else if (checkstring(argv[4], "O")) {
        type = kSoundTypeNull;
    } else if (checkstring(argv[4], "P")) {
        type = kSoundTypePeriodicNoise;
    } else if (checkstring(argv[4], "Q")) {
        type = kSoundTypeSquare;
    } else if (checkstring(argv[4], "S")) {
        type = kSoundTypeSine;
    } else if (checkstring(argv[4], "T")) {
        type = kSoundTypeTriangular;
    } else if (checkstring(argv[4], "W")) {
        type = kSoundTypeSawTooth;
    } else {
        char *s = getCstring(argv[4]);
        if (strcasecmp("N", s) == 0) {
            type = kSoundTypeWhiteNoise;
        } else if (strcasecmp("O", s) == 0) {
            type = kSoundTypeNull;
        } else if (strcasecmp("P", s) == 0) {
            type = kSoundTypePeriodicNoise;
        } else if (strcasecmp("Q", s) == 0) {
            type = kSoundTypeSquare;
        } else if (strcasecmp("S", s) == 0) {
            type = kSoundTypeSine;
        } else if (strcasecmp("T", s) == 0) {
            type = kSoundTypeTriangular;
        } else if (strcasecmp("W", s) == 0) {
            type = kSoundTypeSawTooth;
        } else {
            MMRESULT_RETURN_EX(kSyntax, "Sound type must be N, O, P, Q, S, T or W");
        }
        ClearSpecificTempMemory(s);
    }

    if (type != kSoundTypeNull && argc < 7) return kArgumentCount;

    float frequency = has_arg(6) ? (float)getnumber(argv[6]) : 0.0;
    int volume = has_arg(8) ? getint(argv[8], 0, 100 / MAXSOUNDS) : 100 / MAXSOUNDS;

    return audio_play_sound(sound_no, channel, type, frequency, volume);
}

/** PLAY STOP */
static MmResult cmd_play_stop(const char *p) {
    skipspace(p);
    if (!parse_is_end(p)) return kUnexpectedText;
    return audio_stop();
}

/** PLAY TONE left [, right] [, dur] [, interrupt] */
static MmResult cmd_play_tone(const char *p) {
    getargs(&p, 7, ",");
    if (argc != 3 && argc != 5 && argc != 7) return kArgumentCount;
    float f_left = (float)getnumber(argv[0]);
    float f_right = (float)getnumber(argv[2]);
    int64_t duration = (argc > 4) ? getint(argv[4], 0, INT_MAX) : -1;
    const char *interrupt = (argc == 7) ? GetIntAddress(argv[6]) : NULL;

    return audio_play_tone(f_left, f_right, duration, interrupt);
}

static MmResult cmd_play_stream(const char *p) {
    ERROR_UNIMPLEMENTED("PLAY STREAM");
    return kUnimplemented;
}

/** PLAY MP3 file$ [, interrupt] */
static MmResult cmd_play_mp3(const char *p) {
    getargs(&p, 3, ",");
    if (argc != 1 && argc != 3) return kArgumentCount;

    const char *file = getCstring(argv[0]);
    const char *interrupt = (argc > 1) ? GetIntAddress(argv[2]) : NULL;

    return audio_play_mp3(file, interrupt);
}

/** PLAY VOLUME left [, right] */
static MmResult cmd_play_volume(const char *p) {
    getargs(&p, 3, ",");
    if (argc != 1 && argc != 3) return kArgumentCount;
    uint8_t left = (uint8_t)getint(argv[0], 0, 100);
    uint8_t right = (argc == 3) ? (uint8_t)getint(argv[2], 0, 100) : left;

    return audio_set_volume(left, right);
}

/** PLAY WAV file$ [, interrupt] */
static MmResult cmd_play_wav(const char *p) {
    getargs(&p, 3, ",");
    if (argc != 1 && argc != 3) return kArgumentCount;

    const char *file = getCstring(argv[0]);
    const char *interrupt = (argc > 1) ? GetIntAddress(argv[2]) : NULL;

    return audio_play_wav(file, interrupt);
}

void cmd_play(void) {
    MmResult result = kOk;
    const char *p;
    if ((p = checkstring(cmdline, "CLOSE"))) {
        result = cmd_play_stop(p); // CLOSE is a synonym of STOP.
    } else if ((p = checkstring(cmdline, "CONTINUE"))) {
        result = cmd_play_continue(p);
    } else if ((p = checkstring(cmdline, "CONTINUE"))) {
        result = cmd_play_continue(p);
    } else if ((p = checkstring(cmdline, "EFFECT"))) {
        result = cmd_play_effect(p);
    } else if ((p = checkstring(cmdline, "FLAC"))) {
        result = cmd_play_flac(p);
    } else if ((p = checkstring(cmdline, "HALT"))) {
        result = cmd_play_halt(p);
    } else if ((p = checkstring(cmdline, "LOAD SOUND"))) {
        result = cmd_play_load_sound(p);
    } else if ((p = checkstring(cmdline, "MIDI"))) {
        result = cmd_play_midi(p);
    } else if ((p = checkstring(cmdline, "MIDIFILE"))) {
        result = cmd_play_midifile(p);
    } else if ((p = checkstring(cmdline, "MODFILE"))) {
        result = cmd_play_modfile(p);
    } else if ((p = checkstring(cmdline, "MODSAMPLE"))) {
        result = cmd_play_modsample(p);
    } else if ((p = checkstring(cmdline, "MP3"))) {
        result = cmd_play_mp3(p);
    } else if ((p = checkstring(cmdline, "NEXT"))) {
        result = cmd_play_next(p);
    } else if ((p = checkstring(cmdline, "NOTE"))) {
        result = cmd_play_note(p);
    } else if ((p = checkstring(cmdline, "PAUSE"))) {
        result = cmd_play_pause(p);
    } else if ((p = checkstring(cmdline, "PREVIOUS"))) {
        result = cmd_play_previous(p);
    } else if ((p = checkstring(cmdline, "RESUME"))) {
        result = cmd_play_resume(p);
    } else if ((p = checkstring(cmdline, "SOUND"))) {
        result = cmd_play_sound(p);
    } else if ((p = checkstring(cmdline, "STOP"))) {
        result = cmd_play_stop(p);
    } else if ((p = checkstring(cmdline, "STREAM"))) {
        result = cmd_play_stream(p);
    } else if ((p = checkstring(cmdline, "TONE"))) {
        result = cmd_play_tone(p);
    } else if ((p = checkstring(cmdline, "VOLUME"))) {
        result = cmd_play_volume(p);
    } else if ((p = checkstring(cmdline, "WAV"))) {
        result = cmd_play_wav(p);
    } else {
        ERROR_UNKNOWN_SUBCOMMAND("PLAY");
    }
    ERROR_ON_FAILURE(result);
}
