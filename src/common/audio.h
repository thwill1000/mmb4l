/*-*****************************************************************************

MMBasic for Linux (MMB4L)

audio.h

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

#if !defined(MMBASIC_AUDIO_H)
#define MMBASIC_AUDIO_H

#include "mmresult.h"

#include <stdbool.h>
#include <stdint.h>

#define MAXSOUNDS  4

typedef enum {
   kChannelLeft = 0x1,
   kChannelRight = 0x2,
   kChannelBoth = 0x3
} Channel;

typedef enum {
  kSoundTypeSine,
  kSoundTypeSquare,
  kSoundTypeTriangular,
  kSoundTypeSawTooth,
  kSoundTypePeriodicNoise,
  kSoundTypeWhiteNoise,
  kSoundTypeNull
} SoundType;

MmResult audio_init();
const char *audio_last_error();
MmResult audio_close(bool all);

/** Pauses playing audio. */
MmResult audio_pause();

/** Resumes playing audio. */
MmResult audio_resume();

/**
 * Plays a sound indefinitely, up to 4 sounds may be playing simultaneously.
 *
 * @param  sound_no   Which sound: 0-3.
 * @param  channel    Left speaker, right speaker or both.
 * @param  type       Wave type/shape.
 * @param  frequency  Wave frequency
 * @param  volume     Sound volume: 0-25
 */
MmResult audio_play_sound(uint8_t sound_no, Channel channel, SoundType type, float frequency,
                          uint8_t volume);

/**
 * Loads and plays a .mod file asynchronously.
 *
 * @param  filename   The file to load.
 * @param  interrupt  Pointer to optional interrupt routine to call when/if the .mod file ends.
 *                    Use NULL for no interrupt routine.
 */
MmResult audio_play_modfile(const char *filename, const char *interrupt);

/**
 * Plays a sample/effect concurrently with an already playing .mod file.
 *
 * @param  sample_num   Sample number 1 .. 32.
 * @param  channel_num  Channel number 1 .. 4.
 * @param  volume       Volume 0 .. 64.
 * @param  sample_rate
 */
MmResult audio_play_modsample(uint8_t sample_num, uint8_t channel_num, uint8_t volume,
                              uint32_t sample_rate);

/**
 * Plays a sine wave for a given duration.
 *
 * @param  f_left     Frequency to play on the left speaker.
 * @param  f_right    Frequency to play on the right speaker.
 * @param  duration   Duration in milliseconds.
 * @param  interrupt  Optional interrupt to call when duration ends.
 */
MmResult audio_play_tone(float f_left, float f_right, int64_t duration, const char *interrupt);

void audio_service_buffers();

/**
 * Sets the audio volume.
 *
 * @param  left  Volume for left channel 0 .. 100.
 * @param  right  Volume for right channel 0 .. 100.
 */
MmResult audio_set_volume(uint8_t left, uint8_t right);

#endif // #if !defined(MMBASIC_AUDIO_H)
