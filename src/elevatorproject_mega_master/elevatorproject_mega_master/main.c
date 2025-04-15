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
#define DOOR 3
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
	//lcd_puts("Choose floor");
	KEYPAD_Init();
}

bool is_emergency_button_pressed()
{
	//
	return !(PINA & (1 << PA2));		
}



int main(void)
{
    // initial lcd display
	LCD_setup();
	
	// elevator variables 
	static int8_t request_floor = 0 , current_floor = 0;
	
	static bool b_doors_open = false;	
	char req_floor_string[4];
	char curr_floor_string[4];
	char floor_numb_string[5];

	uint8_t floor_numbers[2] = {0xFF,0xFF};
	uint8_t index = 0;
	
	//emergency button
	DDRA &= ~(1 << PA2);
	    
    while (1) 
    {

 		
		
		

		switch (state)
		{
		case IDLE:
			
			lcd_clrscr();
			lcd_puts("Choose floor");
			lcd_gotoxy(0, 1);

			//uint8_t digits[2] = {0xFF, 0xFF};
			uint8_t key_signal_1,key_signal_2 ;
			//char key_signal;
			//int8_t floor_input  ;
			////key_signal = KEYPAD_GetKey();

			do {key_signal_1 = KEYPAD_GetKey();} while(!(key_signal_1 >= '0' && key_signal_1 <= '9'));
			do {key_signal_2 = KEYPAD_GetKey();} while(!(key_signal_2 >= '0' && key_signal_2 <= '9'));
			
				

				
			
				
			//if (floor_input != -1)
		
			
				request_floor = (key_signal_1 -'0') * 10 + ( key_signal_2  -'0');
				lcd_clrscr();
				lcd_puts("Floor: ");
				//itoa(request_floor, req_floor_string, 10);
				lcd_gotoxy(7, 0);
				lcd_putc(key_signal_1);
				lcd_putc(key_signal_2);
				_delay_ms(1000);

				if ((request_floor > 99) || (request_floor < 1))
				{
					state = FAULT;
				}
				else if (request_floor == current_floor)
				{
					state = FAULT;
				}
				else if (request_floor > current_floor)
				{
					state = GOINGUP;
				}
				else if (request_floor < current_floor)
				{
					state = GOINGDOWN;
				}
					
				
			

			break;

		case GOINGUP:
			if (current_floor < request_floor) 
			{
				current_floor++;
				lcd_clrscr();
				lcd_puts("Up");
				lcd_gotoxy(7, 0);
				//itoa(current_floor, curr_floor_string, 10);
				lcd_puts("RF:");
				lcd_putc(request_floor / 10 +'0' );
				//lcd_gotoxy(0,1);
				lcd_putc(request_floor % 10 +'0');
				lcd_gotoxy(0,1);
				lcd_putc(current_floor / 10 +'0' );
				//lcd_gotoxy(0,1);
				lcd_putc(current_floor % 10 +'0');
				_delay_ms(500);
			} 
			else 
			{
				state = DOOR;
			}
			

			//state = elevator_movement(&current_floor,request_floor);
			break;
		case GOINGDOWN:
		// Movement led on 
			//lcd_puts("down");
			//state = elevator_movement(&current_floor,request_floor);
		if (current_floor > request_floor)
		{
			current_floor--;
			lcd_clrscr();
			lcd_puts("Down");
			lcd_gotoxy(7, 0);
			//itoa(current_floor, curr_floor_string, 10);
			lcd_puts("RF:");
			lcd_putc(request_floor / 10 +'0' );
			//lcd_gotoxy(0,1);
			lcd_putc(request_floor % 10 +'0');
			lcd_gotoxy(0,1);
			lcd_putc(current_floor / 10 +'0' );
			//lcd_gotoxy(0,1);
			lcd_putc(current_floor % 10 +'0');
			_delay_ms(500);
		}
		else
		{
			state = DOOR;
		}
		break;
		
		case DOOR:
			lcd_clrscr();
			lcd_puts("door");
			_delay_ms(5000);
			state = IDLE;
		
		// door open led to uno (5 sec)
		// door open 
		// back to idle state = IDLE
		
			
			break;
		
		case FAULT:
			lcd_puts("fault");
		break;
		
		case EMERGENCY:
			lcd_puts("emergency");
			
			
			
			// buzzer on -> plays melody 
		break;
		
		
		}
		
	
	
	}
}

