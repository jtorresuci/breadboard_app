/*
 * Project3.c
 *
 * Created: 2/13/2020 4:30:33 PM
 * Author : JOE TORRES
 */ 

#include <avr/io.h>
#include <stdio.h>
#include "avr.h"
#include "lcd.h"

#define WHOLE_NOTE		1
#define HALF_NOTE		0.5
#define QUARTER_NOTE	0.25
#define EIGHT_NOTE		0.125

#define NUMNOTES sizeof(song)/sizeof(song[0])

//change port to whatever port the speaker is in
#define SPK_ON		SET_BIT(PORTA, 0)
#define SPK_OFF		CLR_BIT(PORTA, 0)

struct note {
	int frequency;
	int avr_arg;
	double duration;
};

char keypad[16] = {
	'1', '2', '3', 'A',
	'4', '5', '6', 'B',
	'7', '8', '9', 'C',
	'*', '0', '#', 'D'
};

int start = 0;
int key_pressed;

const struct note song[] = {
	{554, 90, QUARTER_NOTE},
	{493, 101, QUARTER_NOTE},
	{440, 113, QUARTER_NOTE},
	{493, 101, QUARTER_NOTE},
	{554, 90, QUARTER_NOTE},
	{554, 90, QUARTER_NOTE},
	{554, 90, HALF_NOTE},
	{493, 101, QUARTER_NOTE},
	{493, 101, QUARTER_NOTE},
	{493, 101, HALF_NOTE},
	{554, 90, QUARTER_NOTE},
	{659, 75, QUARTER_NOTE},
	{659, 75, HALF_NOTE},
	{554, 90, QUARTER_NOTE},
	{493, 101, QUARTER_NOTE},
	{440, 113, QUARTER_NOTE},
	{493, 101, QUARTER_NOTE},
	{554, 90, QUARTER_NOTE},
	{554, 90, QUARTER_NOTE},
	{554, 90, QUARTER_NOTE},
	{554, 90, QUARTER_NOTE},
	{493, 101, QUARTER_NOTE},
	{493, 101, QUARTER_NOTE},
	{554, 90, QUARTER_NOTE},
	{493, 101, QUARTER_NOTE},
	{440, 113, WHOLE_NOTE}
};

int is_pressed(int row, int column) {
	// set all pins in PC to N/C
	PORTC = 0x00;
	DDRC = 0x00;
	
	// set rth bit in PC to W1
	CLR_BIT(DDRC, row);
	SET_BIT(PORTC, row);
	
	// set cth bit in PC to 0
	SET_BIT(DDRC, column);
	CLR_BIT(PORTC, column);
	
	// if(rth bit in PC is 0), return 1
	if(!GET_BIT(PINC, row)) {
		return 1;
	}
	
	// else return 0
	return 0;
}


int get_key() {
	int row, col;
	for(row = 0; row < 4; ++row) {
		for(col = 0; col < 4; ++col) {
			if(is_pressed(row, col + 4)) {
				return row * 4 + col + 1;
			}
		}
	}
	return 0;
}

void
avr_wait2(unsigned short msec)
{
	TCCR0 = 3;
	while (msec--) {
		TCNT0 = (unsigned char)(256 - (XTAL_FRQ / 64) * 0.000001);
		SET_BIT(TIFR, TOV0);
		WDR();
		while (!GET_BIT(TIFR, TOV0));
	}
	TCCR0 = 0;
}

/*
void playNote(int frequency, int duration) {
	int k = frequency * duration;
	
	int period = (1 / frequency);
	int t_on = period / 2;
	int t_off = period / 2;

	for(int i = 0; i < k; ++i) {
		SPK_ON;
		avr_wait2(101); // change avr_wait implementation?
		SPK_OFF;
		avr_wait2(101);
	}
}
*/

void playNote(int frequency, int avr_arg, double duration) {
	double k = frequency * duration;
	
	for(int i = 0; i < k; ++i) {
		SPK_ON;
		avr_wait2(avr_arg);
		SPK_OFF;
		avr_wait2(avr_arg);
	}
}

void playSong() {
	for(int i = 0; i < NUMNOTES; ++i) {
		playNote(song[i].frequency, song[i].avr_arg, song[i].duration);
		
		key_pressed = get_key() - 1;
		if(keypad[key_pressed] == '#') {
			start = 0;
			break;
		}
	}
}

int main(void)
{
	//initialize speaker port
	SET_BIT(DDRA, 0); // set DDR to whatever port the speaker is in
	lcd_init();
	lcd_pos(0, 0);
	lcd_puts2("SONG: MARY HAD..");
	lcd_pos(1, 0);
	lcd_puts2("VOLUME: MIDDLE");
	
	while (1)
	{
		key_pressed = get_key() - 1;
		
		if(keypad[key_pressed] == '*') {
			start = 1;
		}
		
		if(start == 1) {
			playSong();
		}
	}
}
