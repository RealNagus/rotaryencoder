/*
 *  rotenc.h
 *  This file is part of RotaryEncoder.
 *
 *  RotaryEncoder is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  your option) any later version.
 *
 *  RotaryEncoder is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with RotaryEncoder.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 *  Created:  Oct 10, 2015
 *  Author:   Thomas Heßling <mail@dream-dimensions.de>
 *
 */

/*
 * This code is just a simple demonstration of how you can use a rotary encoder
 * with software debounce. Note, that the debounce is only used when waiting for
 * the first event (pin change), not the second. This implementation is,
 * however, straight forward and left as an exercise for the reader/user. ;-)
 *
 */


#ifndef ROTENC_H_
#define ROTENC_H_

#include <stdbool.h>

// port/pin configurations. Change them to match you circuit.
// ROTENC_DEBOUNCE sets the number of debounce waiting cycles.
#define ROTENC_DDR			DDRC
#define ROTENC_PORT			PORTC
#define ROTENC_PIN			PINC
#define	ROTENC_PIN_A		PC0
#define ROTENC_PIN_B		PC1
#define ROTENC_DEBOUNCE		1


/*  Nothing to change beyond this line. */


// the different possible states
#define ROTENC_STATE_WAIT			0
#define ROTENC_STATE_WAIT_ASTAB		1
#define ROTENC_STATE_WAIT_BSTAB		2
#define ROTENC_STATE_WAIT_A			4
#define ROTENC_STATE_WAIT_B			8
#define ROTENC_STATE_ROT_CW			16
#define ROTENC_STATE_ROT_CCW		32


// helper struct to store position, state etc.
typedef struct {
	 uint8_t state;
	 uint8_t debounce_count;
	 uint16_t position;
} rotenc_state;


void rotenc_init();
void rotenc_update();
void rotenc_inc_position();
void rotenc_dec_position();
uint8_t rotenc_get_state();
uint16_t rotenc_get_position();
void rotenc_set_position(uint16_t pos);


#endif /* ROTENC_H_ */
