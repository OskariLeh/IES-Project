/*
 * elevatorproject_mega_master.c
 *
 * Created: 31/03/2025 10:16:43
 * Author : Akseli, Martta, Oskari, Wenhao and Wenjun
 */ 

#define F_CPU 16000000UL
#define BAUD 9600


#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include <util/setbaud.h>
#include <stdbool.h>
#include "lcd.h"
#include "keypad.h"

// states
#define IDLE 0
#define GOINGUP 1
#define GOINGDOWN 2
#define DOOROPEN 3
#define FAULT 4
#define EMERGENCY 5
volatile int8_t state = 0;

/* Notes


External button pin 22 - PA2
Display current floor 

**Switch states case frame.** 

In each case send SPI message to salve UNO to indicate elevator moving, door opening and emergency. 
Each state should check if emergency button is pressed.

Fault state checks if current and requested floor is the same.

**End of switch states case**



*/


//LCD display setup
void LCD_setup()
{
	lcd_init(LCD_DISP_ON);
	lcd_clrscr();
	lcd_puts("Ready?");
	KEYPAD_Init();
}

int main(void)
{
    // initial lcd display
	LCD_setup();
	
	// elevator variables 
	static int8_t request_floor = 1;
	static int8_t current_floor = 1;
	static bool b_doors_open = false;	
	
    while (1) 
    {
		static int8_t request_floor = 1;
		static int8_t current_floor = 1;
		static bool b_doors_open = false;
 		uint8_t key_signal = KEYPAD_GetKey();
		_delay_ms(300);
		
		// Check if floor button is pressed
		if(key_signal != 0xFF)
		{
			char key_str[4];
			
			if(key_signal >= '1' && key_signal <= '9')
			{
				
				//convert ascii to numeric value
				uint8_t key_value = key_signal -'0';
				// numeric to string
				itoa(key_value, key_str, 10);
			}
			else
			{
				//Special keys
				key_str[0] = key_signal;
				key_str[1] = '\0';
			}
			lcd_clrscr();
			lcd_puts("“Choose the floor:");
			lcd_gotoxy(0,1);
			lcd_puts(key_str);
		}
		
		
    }
}

