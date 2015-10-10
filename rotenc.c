/*
 *  rotenc.c
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


#include <util/atomic.h>

#include "rotenc.h"


// a local static variable
static rotenc_state re;


/*
 * Initialize the ports etc.
 */
void
rotenc_init()
{
	// configure port direction
	ROTENC_DDR &= ~((1<<ROTENC_PIN_A)|(1<<ROTENC_PIN_B));
	// activate internal pull ups
	ROTENC_PORT |= (1<<ROTENC_PIN_A)|(1<<ROTENC_PIN_B);

	re.position = 0;
	re.state = ROTENC_STATE_WAIT;
	re.debounce_count = 0;
}


/*
 * This is the magic part where we check the pins and act according to the
 * current state we're in.
 */
void
rotenc_update()
{
	uint8_t pinA, pinB;

	// invert signals, they are active low
	pinA = !(ROTENC_PIN & (1<<ROTENC_PIN_A));
	pinB = !(ROTENC_PIN & (1<<ROTENC_PIN_B));

	// if both are FALSE, return to waiting state
	if (! pinA && ! pinB)
	{
		re.state = ROTENC_STATE_WAIT;
		re.debounce_count = 0;
	}

	// initial state, we are waiting for something to happen
	if (re.state == ROTENC_STATE_WAIT)
	{
		// only one pin can be positive here, otherwise something
		// weird happened (bounce, noise, etc.)
		if (pinA && ! pinB)
			re.state = ROTENC_STATE_WAIT_ASTAB;
			//re.state = ROTENC_STATE_WAIT_B;
		else if (! pinA && pinB)
			re.state = ROTENC_STATE_WAIT_BSTAB;
			//re.state = ROTENC_STATE_WAIT_A;
	}

	// we are waiting for pin A to stabilize, just count up until
	// the limit is reached and check the status of pin A again
	if (re.state == ROTENC_STATE_WAIT_ASTAB)
	{
		if (re.debounce_count < ROTENC_DEBOUNCE)
			re.debounce_count++;
		else
		{
			// there is only valid state here (pin A low/true and B high/false)
			if (pinA && ! pinB)
				re.state = ROTENC_STATE_WAIT_B;
			else
				re.state = ROTENC_STATE_WAIT;
		}
	}

	// waiting for stabilization of pin B is equivalent
	if (re.state == ROTENC_STATE_WAIT_BSTAB)
	{
		if (re.debounce_count < ROTENC_DEBOUNCE)
			re.debounce_count++;
		else
		{
			// there is only valid state here (pin A low/true and B high/false)
			if (! pinA && pinB)
				re.state = ROTENC_STATE_WAIT_A;
			else
				re.state = ROTENC_STATE_WAIT;
		}
	}

	// if we got this far and both pins are true/low we got a completed step, either
	// CW or CCW depending on the wait state
	if (re.state == ROTENC_STATE_WAIT_A && pinA && pinB)
	{
		re.state = ROTENC_STATE_ROT_CCW;
		rotenc_dec_position();
	}

	if (re.state == ROTENC_STATE_WAIT_B && pinA && pinB)
	{
		re.state = ROTENC_STATE_ROT_CW;
		rotenc_inc_position();
	}

}


/*
 * manually increase the current position by one
 */
void
rotenc_inc_position()
{
	// only increase up to max value possible
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if (re.position < 0xffff)
			re.position++;
	}
}

/*
 * manually decrease the current position by one
 */
void
rotenc_dec_position()
{
	// only decrease down to 0
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if (re.position > 0)
			re.position--;
	}
}

/*
 * query the state (you don't really need this...)
 */
uint8_t
rotenc_get_state()
{
	return re.state;
}

/*
 * query the current position
 */
uint16_t
rotenc_get_position()
{
	return re.position;
}

/*
 * set the position to an arbitrary value
 */
void
rotenc_set_position(uint16_t pos)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		re.position = pos;
	}
}
