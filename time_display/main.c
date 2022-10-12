/*
 * Project2.c
 *
 * Created: 1/29/2020 2:48:45 PM
 * Author : Reniel
 */ 

#include <avr/io.h>
#include <stdio.h>
#include "avr.h"
#include "lcd.h"

char keypad[16] = {
	'1', '2', '3', 'A',
	'4', '5', '6', 'B',
	'7', '8', '9', 'C',
	'*', '0', '#', 'D'
};

int days_in_month[12] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

struct time {
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
	int ms;
} time;

int blink = 0; //boolean to make colon blink after each cycle
int military_time = 0; //boolean to switch to military time
int key_pressed; //number corresponding to key pressed
int edit_mode = 0; //boolean for switching to edit mode
int edit_pos = 0; //index for what user wants to edit; 1 = month, 2 = day, 3 = year, etc.

/* CODE FOR LED
void blink() {
	SET_BIT(PORTB, 0);
	avr_wait(500);
	CLR_BIT(PORTB, 0);
	avr_wait(500);
}

void blinkNtimes(int n) {
	for(int i = 0; i < n; ++i) {
		blink();
	}
}
*/

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

void display_datetime(struct time * t) {
	lcd_clr();
	
	char buf[17]; // buffer for one row of char, 16 characters in display + 1 null terminating character
	
	//display date in the top row MM/DD/YYYY
	sprintf(buf, "%02d/%02d/%04d", t->month + 1, t->day, t->year);
	
	lcd_pos(0, 0);
	lcd_puts2(buf);
	
	if(military_time) {
		if(blink) {
			sprintf(buf, "%02d:%02d:%02d", t->hour, t->minute, t->second);
		} else {
			sprintf(buf, "%02d %02d %02d", t->hour, t->minute, t->second);
		}
	} else {
	
		//display time on the bottom row HH:MM:SS
		int period; // AM = 0; PM = 1
		char * ampm[2] = {"AM", "PM"};
	
		if(t->hour < 12) {
			period = 0;
		} else {
			period = 1;
		}
	
		int twelve_hour_time;
	
		if(t->hour % 12 == 0) {
			twelve_hour_time = 12;
		} else {
			twelve_hour_time = t->hour % 12;
		}
	
		if(blink) {
			if(period == 0) {
				sprintf(buf, "%02d:%02d:%02d %s", twelve_hour_time, t->minute, t->second, ampm[period]);
			} else {
				sprintf(buf, "%02d:%02d:%02d %s", twelve_hour_time, t->minute, t->second, ampm[period]);
			}
		} else {
			if(period == 0) {
				sprintf(buf, "%02d %02d %02d %s", twelve_hour_time, t->minute, t->second, ampm[period]);
			} else {
				sprintf(buf, "%02d %02d %02d %s", twelve_hour_time, t->minute, t->second, ampm[period]);
			}
		}
	}
	
	lcd_pos(1, 0);
	lcd_puts2(buf);
}

void advance_time(struct time * t) {
	++t->ms;

	if(t->ms >= 10) {
		t->ms = 0;
		++t->second;
	}
	
	if(t->second >= 60) {
		t->second = 0;
		++t->minute;
	}
	
	if(t->minute >= 60) {
		t->minute = 0;
		++t->hour;
	}
	
	if(t->hour >= 24) {
		t->hour = 0;
		++t->day;
	}
	
	//handle the case when it's February and it's a leap year
	if(t->month == 1 && t->year % 4 == 0 && (t->year % 100 != 0 || t->year % 400 == 0) && t->month == 1 && t->day == 29) {
		return;
	}
	
	if(t->day > days_in_month[t->month]) {
		t->day = 1;
		++t->month;
	}
	
	if(t->month >= 12) {
		t->month = 0;
		++t->year;
	}
}

void add_time(struct time * t) {
	if(edit_pos == 1) {
		++t->month;
		t->month = t->month % 12;
	} else if(edit_pos == 2) {
		++t->day;
		if(t->month == 1 && t->year % 4 == 0 && (t->year % 100 != 0 || t->year % 400 == 0) && t->day == 29) {
			t->day = 29;
		} else if(t->day > days_in_month[t->month]) {
			t->day = 1;
		}
	} else if(edit_pos == 3) {
		++t->year;
	} else if(edit_pos == 4) {
		++t->hour;
		if(t->hour > 24) {
			t->hour = 1;
		}
	} else if(edit_pos == 5) {
		++t->minute;
		if(t->minute > 59) {
			t->minute = 0;
		}
	} else if(edit_pos == 6) {
		++t->second;
		if(t->second > 59) {
		t->second = 0;
		}
	}

	display_datetime(t);
	avr_wait(500);
}

void subtract_time(struct time * t) {
	if(edit_pos == 1) {
		--t->month;
		if(t->month < 0) {
			t->month = 11;
		}
	} else if(edit_pos == 2) {
		--t->day;
		if(t->day < 1) {
			if(t->month == 1 && t->year % 4 == 0 && (t->year % 100 != 0 || t->year % 400 == 0)) {
				t->day = 29;
			} else {
				t->day = days_in_month[t->month];
			}
		}
	} else if(edit_pos == 3) {
		--t->year;
	} else if(edit_pos == 4) {
		--t->hour;
		if(t->hour < 1) {
			t->hour = 24;
		}
	} else if(edit_pos == 5) {
		--t->minute;
		if(t->minute < 0) {
			t->minute = 59;
		}
	} else if(edit_pos == 6) {
		--t->second;
		if(t->second < 0) {
			t->second = 59;
		}
	}
	
	display_datetime(t);
	avr_wait(500);
}

void change_lcd_edit_tag() {
	if(edit_pos == 1) {
		lcd_pos(1, 13);
		lcd_puts2(":MM");
	} else if(edit_pos == 2) {
		lcd_pos(1, 13);
		lcd_puts2(":DD");
	} else if(edit_pos == 3) {
		lcd_pos(1, 13);
		lcd_puts2(":YY");
	} else if(edit_pos == 4) {
		lcd_pos(1, 13);
		lcd_puts2(":HH");
	} else if(edit_pos == 5) {
		lcd_pos(1, 13);
		lcd_puts2(":MM");
	} else if(edit_pos == 6) {
		lcd_pos(1, 13);
		lcd_puts2(":SS");
	}
}
	
int main(void)
{
	//initialize lcd
	lcd_init();
	
	//initialize time
	//year, month, day, hour, minute, second
	struct time t = {2020, 11, 31, 13, 59, 55, 0};
	
	while (1)
	{
		key_pressed = get_key() - 1;
		
		if(key_pressed == -1) {
			//don't do anything
		} else if(keypad[key_pressed] == 'A' && edit_mode == 0) {
			//A - switch to edit mode
			edit_mode = 1;
		} else if(keypad[key_pressed] == 'B' && edit_mode == 1) {
			//B - which out of edit mode
			edit_mode = 0;
		} else if(keypad[key_pressed] == 'C') {
			//C - set to military time
			military_time = 1;
		} else if(keypad[key_pressed] == 'D') {
			//D - set to AM/PM
			military_time = 0;
		}
		
		if(edit_mode) {
			lcd_pos(0, 12);
			lcd_puts2("EDIT");
			
			//change edit_pos depending on what user wants to edit
			if(keypad[key_pressed] == '1') {
				edit_pos = 1; //edit month
			} else if(keypad[key_pressed] == '2') {
				edit_pos = 2; //edit day
			} else if(keypad[key_pressed] == '3') {
				edit_pos = 3; //edit year
			} else if(keypad[key_pressed] == '4') {
				edit_pos = 4; //edit hour
			} else if(keypad[key_pressed] == '5') {
				edit_pos = 5; //edit minute
			} else if(keypad[key_pressed] == '6') {
				edit_pos = 6; //edit second
			}
			
			//add time
			if(keypad[key_pressed] == '#') {
				add_time(&t);
			}
			
			//subtract time
			if(keypad[key_pressed] == '*') {
				subtract_time(&t);
			}
			
			//show on the lcd what aspect of time the user is editing
			change_lcd_edit_tag();
			
		} else {
			//change blink boolean after each cycle
			if(blink == 0) {
				blink = 1;
			} else {
				blink = 0;
			}
			
			display_datetime(&t);
			advance_time(&t);
			avr_wait(100);
		}
	}
}

