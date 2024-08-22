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

/** Initialised the audio module. */
MmResult audio_init();

/** Terminates the audio module. */
MmResult audio_term();

/** Stops all playing audio. */
MmResult audio_stop();

/** Perform audio background tasks, e.g. keeping the sound buffers filled. */
MmResult audio_background_tasks();

/** Gets the last (presumably audio related) error message from the SDL API */
const char *audio_last_error();

/** Plays next FLAC, MP3 or WAV track. */
MmResult audio_play_next();

/** Pauses playing audio. */
MmResult audio_pause();

/** Plays previous FLAC, MP3 or WAV track. */
MmResult audio_play_previous();

/** Resumes playing audio. */
MmResult audio_resume();

/**
 * Loads and play a .wav file at the same time as a .mod file is playing.
 *
 * If a previous effect file is playing this will immediately terminate it and commence playing
 * the new file.
 *
 * NOTE: .wav files must have the same sample rate as the .mod file output.
 *       Files can be mono or stereo.
 *
 * @param  filename   The file to load.
 * @param  interrupt  Pointer to optional interrupt routine to call when/if the .wav file ends.
 *                    Use NULL for no interrupt routine.
 */
MmResult audio_play_effect(const char *filename, const char *interrupt);

/**
 * Plays a simple sound at a given frequency.
 * Up to 4 sound may be played simultaneously.
 *
 * @param  sound_no   The sound to play 1 .. 4.
 * @param  channel    Left, right or stereo.
 * @param  type       The type of waveform to play.
 * @param  frequency  The frequency to play.
 * @param  volume     Volume 1 .. 25.
 */
MmResult audio_play_sound(uint8_t sound_no, Channel channel, SoundType type, float frequency,
                          uint8_t volume);

/**
 * Loads and plays a .mod file asynchronously.
 *
 * @param  filename     The file to load.
 * @param  sample_rate  Sample rate to play the MOD file at.
 * @param  interrupt    Pointer to optional interrupt routine to call when/if the .mod file ends.
 *                      Use NULL for no interrupt routine.
 */
MmResult audio_play_modfile(const char *filename, unsigned sample_rate, const char *interrupt);

/**
 * Plays a sample/effect concurrently with an already playing .mod file.
 *
 * @param  sample_num   Sample number 1 .. 32.
 * @param  channel_num  Channel number 1 .. 4.
 * @param  volume       Volume 0 .. 64.
 * @param  sample_rate  Sample rate to play the effect at.
 */
MmResult audio_play_modsample(uint8_t sample_num, uint8_t channel_num, uint8_t volume,
                              unsigned sample_rate);

/**
 * Plays a simple sine-wave tone.
 *
 * @param  f_left     Frequency to play on left channel.
 * @param  f_right    Frequency to play on right channel.
 * @param  duration   Duration in milliseconds to play the tone for.
 * @param  interrupt  Pointer to optional interrupt routine to call when the duration ends.
 *                    Use NULL for no interrupt routine.
 */
MmResult audio_play_tone(float f_left, float f_right, int64_t duration, const char *interrupt);

/**
 * Loads and plays a .flac file asynchronously.
 *
 * @param  filename   The file to load.
 * @param  interrupt  Pointer to optional interrupt routine to call when/if the .mp3 file ends.
 *                    Use NULL for no interrupt routine.
 */
MmResult audio_play_flac(const char *filename, const char *interrupt);

/**
 * Loads and plays a .mp3 file asynchronously.
 *
 * @param  filename   The file to load.
 * @param  interrupt  Pointer to optional interrupt routine to call when/if the .mp3 file ends.
 *                    Use NULL for no interrupt routine.
 */
MmResult audio_play_mp3(const char *filename, const char *interrupt);

/**
 * Loads and plays a .wav file asynchronously.
 *
 * @param  filename   The file to load.
 * @param  interrupt  Pointer to optional interrupt routine to call when/if the .wav file ends.
 *                    Use NULL for no interrupt routine.
 */
MmResult audio_play_wav(const char *filename, const char *interrupt);

/**
 * Sets the audio volume.
 *
 * @param  left  Volume for left channel 0 .. 100.
 * @param  right  Volume for right channel 0 .. 100.
 */
MmResult audio_set_volume(uint8_t left, uint8_t right);

#endif // #if !defined(MMBASIC_AUDIO_H)
