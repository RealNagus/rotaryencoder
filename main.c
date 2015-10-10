/*
 *	main.c
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
 * This program is indented for demonstration purposes only.
 * All it accomplishes is running a timer, querying the rotary encoder and
 * sending the current position via uart.
 *
 * To verify the position you have to connect the circuit via uart to a
 * computer, e.g. with a FT232-type uart-to-USB converter.
 *
 */



#include <avr/io.h>
#include <avr/iom168.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "rotenc.h"


#ifndef F_CPU
#define F_CPU 16000000UL
#endif



// uart baudrate
#define BAUD 250000L

// verify baud rate setting (taken from the http://www.mikrocontroller.net forums/articles)
#define UBRR_VAL ((F_CPU+BAUD*8)/(BAUD*16)-1)   // smart round
#define BAUD_REAL (F_CPU/(16*(UBRR_VAL+1)))     // real baudrate
#define BAUD_ERROR ((BAUD_REAL*1000)/BAUD) 		// error in promille; 1000 = no error

#if ((BAUD_ERROR<990) || (BAUD_ERROR>1010))
#error Error. Your baudrate error exceeds 1%!
#endif


#include <util/setbaud.h>



/* UART-Init ATmega16 */
void uart_init(void)
{
    UBRR0H = (uint8_t) UBRR_VAL >> 8;
    UBRR0L = (uint8_t) UBRR_VAL & 0xFF;

    UCSR0B |= (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
    UCSR0C |= (1<<UCSZ00)|(1<<UCSZ01);
}

void uart_send_char (unsigned char data)
{
    while (! ( UCSR0A & (1<<UDRE0)));
    UDR0 = data;
}

void uart_send_string (char *data) {
	while (*data) {
		uart_send_char(*data);
		data++;
	}
}

unsigned char uart_receive (void)
{
    while (!(UCSR0A & (1<<RXC0)));
    return UDR0;
}






/*
 * When the timer fires we simply call the rotary encoder's update function
 */
ISR (TIMER0_COMPA_vect)
{
	rotenc_update();
}


void Timer0Start()
{
	// setting for ~ 200 Hz
	TCCR0A = 0;
	TIMSK0 |= (1<<OCIE0A);
	TCCR0B |= (1<<CS01)|(1<<CS00);
}


int
main(void)
{
	// initialization code for various things
	set_sleep_mode(SLEEP_MODE_IDLE);
	sleep_enable();

	uart_init();
	rotenc_init();
	sei();

	Timer0Start();

	// position and uart-string variables
	uint16_t pos, ppos = 0;
	char sdata[50];

	// loop indefinitely
	for(;;)
	{
		// enter sleep mode, which we will wake up from when the timer triggers
		sleep_cpu();

		// query the position and send it via uart (if different from current)
		pos = rotenc_get_position();
		if (pos != ppos)
		{
			snprintf(sdata, 49, "%d\n", pos);
			uart_send_string(sdata);
			ppos = pos;
		}
	}

	return 0;
}

