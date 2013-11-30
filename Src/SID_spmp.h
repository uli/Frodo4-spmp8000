/*
 *  SID_linux.h - 6581 emulation, Linux specific stuff
 *
 *  Frodo Copyright (C) Christian Bauer
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <libgame.h>
#include "VIC.h"

/*
 *  Initialization
 */

emu_sound_params_t sp;
#define SOUND_CHANNELS 1

#define sndbufsize 512
int16 sound_buffer[2][sndbufsize];

void DigitalRenderer::init_sound(void)
{
    sp.rate = SAMPLE_FREQ;
    sp.channels = SOUND_CHANNELS;
    sp.depth = 0;	/* 0 seems to mean 16 bits */
    sp.callback = 0;	/* not used for native games, AFAIK */
    sp.buf_size = sndbufsize * 2;
    (void)emuIfSoundInit(&sp);
	ready = true;
}


/*
 *  Destructor
 */

DigitalRenderer::~DigitalRenderer()
{
    emuIfSoundCleanup();
}


/*
 *  Pause sound output
 */

void DigitalRenderer::Pause(void)
{
}


/*
 * Resume sound output
 */

void DigitalRenderer::Resume(void)
{
}


/*
 * Fill buffer, sample volume (for sampled voice)
 */

void DigitalRenderer::EmulateLine(void)
{
	static int divisor = 0;
	static int to_output = 0;
	static int buffer_pos = 0;
	static int bufnum = 0;

	if (!ready)
	  return;

        sample_buf[sample_in_ptr] = volume;
        sample_in_ptr = (sample_in_ptr + 1) % SAMPLE_BUF_SIZE;

	// Now see how many samples have to be added for this line
	divisor += SAMPLE_FREQ;
	while (divisor >= 0)
		divisor -= TOTAL_RASTERS*SCREEN_FREQ, to_output++;

	// Calculate the sound data only when we have enough to fill
	// the buffer entirely
	if ((buffer_pos + to_output) >= sndbufsize) {
		int datalen = sndbufsize - buffer_pos;
		to_output -= datalen;
		calc_buffer(sound_buffer[bufnum] + buffer_pos, datalen*2);
		sp.buf = (uint8*)sound_buffer[bufnum];
		sp.buf_size = sndbufsize * 2;
		emuIfSoundPlay(&sp);
		buffer_pos = 0;
		/* XXX: cargo cult programming; not sure if the emuIf copies
		   the sound buffer or not, so I use two so as not to
		   overwrite the currently playing buffer... */
		bufnum = !bufnum;
	}
}
