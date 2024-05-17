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

static void cmd_play_continue(const char *p) { ERROR_UNIMPLEMENTED("PLAY CONTINUE"); }

static void cmd_play_effect(const char *p) { ERROR_UNIMPLEMENTED("PLAY EFFECT"); }

static void cmd_play_flac(const char *p) { ERROR_UNIMPLEMENTED("PLAY FLAC"); }

static void cmd_play_halt(const char *p) { ERROR_UNIMPLEMENTED("PLAY HALT"); }

static void cmd_play_load_sound(const char *p) { ERROR_UNIMPLEMENTED("PLAY LOAD SOUND"); }

static void cmd_play_midi(const char *p) { ERROR_UNIMPLEMENTED("PLAY MIDI"); }

static void cmd_play_midifile(const char *p) { ERROR_UNIMPLEMENTED("PLAY MIDIFILE"); }

/** PLAY MODFILE file$ [,interrupt] */
static void cmd_play_modfile(const char *p) {
    getargs(&p, 3, ",");
    if (argc != 1 && argc != 3) ERROR_ARGUMENT_COUNT;
    const char *filename = getCstring(argv[0]);
    const char *interrupt = (argc == 3) ? GetIntAddress(argv[2]) : NULL;

    MmResult result = audio_play_modfile(filename, interrupt);
    if (FAILED(result)) error_throw(result);
}

/** PLAY MODSAMPLE sample_num, channel_num [, volume] [, sample_rate] */
static void cmd_play_modsample(const char *p) {
    getargs(&p, 7, ",");
    if (argc != 3 && argc != 5 && argc != 7) ERROR_ARGUMENT_COUNT;
    const uint8_t sample_num = (uint8_t) getint(argv[0], 1, 32);
    const uint8_t channel_num = (uint8_t) getint(argv[2], 1, 4);
    const uint8_t volume = max(0, (argc >= 5) ? (uint8_t) getint(argv[4], 0, 64) : 64);
    const uint32_t sample_rate = (argc == 7) ? (uint32_t) getint(argv[6], 150, 500000) : 16000;

    MmResult result = audio_play_modsample(sample_num, channel_num, volume, sample_rate);
    if (FAILED(result)) error_throw(result);
}

static void cmd_play_next(const char *p) { ERROR_UNIMPLEMENTED("PLAY NEXT"); }

static void cmd_play_pause(const char *p) { 
    if (!parse_is_end(p)) error_throw(kUnexpectedText);
    MmResult result = audio_pause();
    if (FAILED(result)) error_throw(result);
}

static void cmd_play_previous(const char *p) { ERROR_UNIMPLEMENTED("PLAY PREVIOUS"); }

static void cmd_play_note(const char *p) { ERROR_UNIMPLEMENTED("PLAY NOTE"); }

static void cmd_play_resume(const char *p) {
    if (!parse_is_end(p)) error_throw(kUnexpectedText);
    MmResult result = audio_resume();
    if (FAILED(result)) error_throw(result);
}

/** PLAY SOUND soundno, channelno, type [, frequency] [, volume] */
static void cmd_play_sound(const char *p) {
	getargs(&p, 9, ",");
	if (argc != 5 && argc != 7 && argc != 9) ERROR_ARGUMENT_COUNT;

    MMINTEGER sound_no = getint(argv[0], 1, MAXSOUNDS) - 1;

    Channel channel = kChannelBoth;
	if (checkstring(argv[2], "L")) {
		channel = kChannelLeft;
	} else if (checkstring(argv[2], "R")) {
		channel = kChannelRight;
	} else if (checkstring(argv[2], "B")) {
		channel = kChannelBoth;
	} else {
        error_throw_ex(kSyntax, "Channel number must be L, R or B");
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
        error_throw_ex(kSyntax, "Sound type must be N, O, P, Q, S, T or W");
    }

    if (type == kSoundTypeNull) {
        if (argc != 5) ERROR_ARGUMENT_COUNT;
    } else if (argc < 7) {
        ERROR_ARGUMENT_COUNT;
    }

    float frequency = argc > 5 ? (float)getnumber(argv[6]) : 0.0;

    MMINTEGER volume = argc > 7 ? getint(argv[8], 0, 100 / MAXSOUNDS) : 100 / MAXSOUNDS;

    MmResult result = audio_play_sound(sound_no, channel, type, frequency, volume);
    if (FAILED(result)) error_throw(result);
}

static void cmd_play_stop(const char *p) {
    if (!parse_is_end(p)) error_throw(kUnexpectedText);
    audio_close(true);
}

/** PLAY TONE left [, right [, dur] [, interrupt]]] */
static void cmd_play_tone(const char *p) {
    getargs(&p, 7, ",");
    if (argc != 3 && argc != 5 && argc != 7) ERROR_ARGUMENT_COUNT;
    float f_left = (float)getnumber(argv[0]);
    float f_right = (float)getnumber(argv[2]);
    int64_t duration = argc > 4 ? getint(argv[4], 0, INT_MAX) : -1;
    const char *interrupt = argc == 7 ? argv[6] : NULL;

    MmResult result = audio_play_tone(f_left, f_right, duration, interrupt);
    if (FAILED(result)) error_throw(result);
}

static void cmd_play_stream(const char *p) { ERROR_UNIMPLEMENTED("PLAY STREAM"); }

static void cmd_play_mp3(const char *p) { ERROR_UNIMPLEMENTED("PLAY MP3"); }

/** PLAY VOLUME left [, right] */
static void cmd_play_volume(const char *p) {
    getargs(&p, 3, ",");
    if (argc != 1 && argc != 3) ERROR_ARGUMENT_COUNT;
    uint8_t left = (uint8_t)getint(argv[0], 0, 100);
    uint8_t right = (argc == 3) ? (uint8_t)getint(argv[2], 0, 100) : left;

    MmResult result = audio_set_volume(left, right);
    if (FAILED(result)) error_throw(result);
}

static void cmd_play_wav(const char *p) { ERROR_UNIMPLEMENTED("PLAY WAV"); }

void cmd_play(void) {
    const char *p;
    if ((p = checkstring(cmdline, "CLOSE"))) {
        cmd_play_stop(p); // CLOSE and STOP are synonyms.
    } else if ((p = checkstring(cmdline, "CONTINUE"))) {
        cmd_play_continue(p);
    } else if ((p = checkstring(cmdline, "EFFECT"))) {
        cmd_play_effect(p);
    } else if ((p = checkstring(cmdline, "FLAC"))) {
        cmd_play_flac(p);
    } else if ((p = checkstring(cmdline, "HALT"))) {
        cmd_play_halt(p);
    } else if ((p = checkstring(cmdline, "LOAD SOUND"))) {
        cmd_play_load_sound(p);
    } else if ((p = checkstring(cmdline, "MIDI"))) {
        cmd_play_midi(p);
    } else if ((p = checkstring(cmdline, "MIDIFILE"))) {
        cmd_play_midifile(p);
    } else if ((p = checkstring(cmdline, "MODFILE"))) {
        cmd_play_modfile(p);
    } else if ((p = checkstring(cmdline, "MODSAMPLE"))) {
        cmd_play_modsample(p);
    } else if ((p = checkstring(cmdline, "MP3"))) {
        cmd_play_mp3(p);
    } else if ((p = checkstring(cmdline, "NEXT"))) {
        cmd_play_next(p);
    } else if ((p = checkstring(cmdline, "NOTE"))) {
        cmd_play_note(p);
    } else if ((p = checkstring(cmdline, "PAUSE"))) {
        cmd_play_pause(p);
    } else if ((p = checkstring(cmdline, "PREVIOUS"))) {
        cmd_play_previous(p);
    } else if ((p = checkstring(cmdline, "RESUME"))) {
        cmd_play_resume(p);
    } else if ((p = checkstring(cmdline, "SOUND"))) {
        cmd_play_sound(p);
    } else if ((p = checkstring(cmdline, "STOP"))) {
        cmd_play_stop(p);
    } else if ((p = checkstring(cmdline, "STREAM"))) {
        cmd_play_stream(p);
    } else if ((p = checkstring(cmdline, "TONE"))) {
        cmd_play_tone(p);
    } else if ((p = checkstring(cmdline, "VOLUME"))) {
        cmd_play_volume(p);
    } else if ((p = checkstring(cmdline, "WAV"))) {
        cmd_play_wav(p);
    } else {
        ERROR_UNKNOWN_SUBCOMMAND("PLAY");
    }
}
