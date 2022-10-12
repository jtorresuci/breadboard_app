/*
 * Project1.c
 *
 * Created: 1/21/2020 3:43:51 PM
 * Author : Reniel
 */ 

#include <avr/io.h>
#include "avr.h"


int main(void)
{
    /* Replace with your application code */
	SET_BIT(DDRB, 0);
	CLR_BIT(DDRB, 1);
    while (1)
    {
		if(!GET_BIT(PINB, 1)) {
			SET_BIT(PORTB, 0);
			avr_wait(500);
			CLR_BIT(PORTB, 0);
			avr_wait(500);
		} else {
			CLR_BIT(PORTB, 0);
		}
    }
}

