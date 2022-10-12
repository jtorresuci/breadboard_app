/*
 * Project4.c
 *
 * Created: 2/28/2020 12:39:43 PM
 * Author : Reniel
            Joe
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

unsigned int get_sample() {
	//configure AVC analog reference
	//configure ADC mux
	DDRA = 0x00;
	CLR_BIT(DDRA, 0);
	CLR_BIT(PORTA, 0);
	
	ADMUX = 0x00;
	CLR_BIT(ADMUX, REFS1);
	SET_BIT(ADMUX, REFS0);
	
	//enable it
	//start conversion
	ADCSRA = 0x00;
	SET_BIT(ADCSRA, ADEN);
	SET_BIT(ADCSRA, ADSC);
	
	//wait for conversion to complete
	while(GET_BIT(ADCSRA, ADSC)) {
	}
	
	return ADC;
}

int get_decimal(float voltage) {
	return (voltage - (int)voltage) * 100;
}

void show_empty() {
	lcd_pos(0, 0);
	lcd_puts2("IN:");
	lcd_pos(0, 8);
	lcd_puts2("MX:--- ");
	lcd_pos(1, 0);
	lcd_puts2("MN:--- ");
	lcd_pos(1, 8);
	lcd_puts2("AV:--- ");
}

int main(void)
{
	lcd_init();
	show_empty();
	int start = 0;
	char buf[4];
	
	float min = 0.0;
	float max = 0.0;
	float sum = 0.0;
	float count = 0.0;
	
	int first_read = 1;
	
    while (1) 
    {
		int key_pressed = get_key() - 1;
		
		//if button is pressed, start collecting voltage
		if(keypad[key_pressed] == 'A') {
			start = 1;
			first_read = 1;
		} else if(keypad[key_pressed] == 'B') {
			start = 0;
			min = 0.0;
			max = 0.0;
			sum = 0.0;
			count = 0.0;
			first_read = 1;
		}

			//get instantaneous voltage
			float voltage = (get_sample() / 1023.0) * 5.0;
			sprintf(buf, "%d.%02d", (int)voltage, get_decimal(voltage));
			lcd_pos(0, 3);
			lcd_puts2(buf);
		if(start) {
			
			//get max voltage
			if(voltage > max) {
				sprintf(buf, "%d.%02d", (int)voltage, get_decimal(voltage));
				lcd_pos(0, 11);
				lcd_puts2(buf);
				max = voltage;
			}
			
			//get min voltage
			if(first_read || voltage < min) {
				if(first_read) {
					first_read = 0;
				}
				
				sprintf(buf, "%d.%02d", (int)voltage, get_decimal(voltage));
				lcd_pos(1, 3);
				lcd_puts2(buf);
				min = voltage;
			}
			
			//get average voltage
			sum += voltage;
			count += 1.0;
			float avg_voltage = sum / count;
			
			sprintf(buf, "%d.%02d", (int)avg_voltage, get_decimal(avg_voltage));
			lcd_pos(1, 11);
			lcd_puts2(buf);
		} else {
			show_empty();
		}
		
		avr_wait(500);
    }
}

