/*
 * elevatorproject_mega_master.c
 *
 * Created: 31/03/2025 10:16:43
 * Author : Akseli, Martta, Oskari, Wenhao and Wenjun
 */ 

#define F_CPU 16000000UL
#define FOSC 16000000UL 
#define BAUD 9600
#define MYUBRR (FOSC/16/BAUD-1)

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


void SPI_MasterInit(void) 
{
	/* Set MOSI and SCK output, all others input */ 
	DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2);
	/* Enable SPI, Master, set clock rate fck/16 */ 
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0); 
}


// datasheet p.193
void SPI_MasterTransmit(int data) 
{
	
	PORTB &= ~(1 << PB0); // master output
	_delay_ms(2);
	/* Start transmission */ 
	SPDR = data; 
	/* Wait for transmission complete */ 
	while(!(SPSR & (1<<SPIF)))
	;
	PORTB |= (1 << PB0);
	_delay_ms(2);
}


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

	return (PINA & (1 << PA0));		
}



int main(void)
{
    // initial lcd display and spi
	LCD_setup();
	SPI_MasterInit();
	
	// elevator floor variables 
	static int8_t request_floor = 0 , current_floor = 0;
	
	//emergency button
	DDRA &= ~(1 << PA0);
	

    while (1) 
    {
 		//mosi
		PORTB &= ~(1 << PB0);
		switch (state)
		{
		case IDLE:
			
			lcd_clrscr();
			lcd_puts("Choose floor");
			lcd_gotoxy(0, 1);
			SPI_MasterTransmit(state);

			//uint8_t digits[2] = {0xFF, 0xFF};

			uint8_t key_signal_1,key_signal_2 ;

			//get keypad inputs
			do {key_signal_1 = KEYPAD_GetKey();} while(!(key_signal_1 >= '0' && key_signal_1 <= '9'));
			do {key_signal_2 = KEYPAD_GetKey();} while(!(key_signal_2 >= '0' && key_signal_2 <= '9'));
			
			
				request_floor = (key_signal_1 -'0') * 10 + ( key_signal_2  -'0');
				lcd_clrscr();
				lcd_puts("Floor: ");
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
			// check if em button is pressed
			if (is_emergency_button_pressed())
			{
				state = EMERGENCY;
				break;
			}		
			SPI_MasterTransmit(state);
			if (current_floor < request_floor) 
			{
				current_floor++;
				lcd_clrscr();
				lcd_puts("Up");
				lcd_gotoxy(7, 0);
				lcd_puts("RF:");
				lcd_putc(request_floor / 10 +'0' );
				lcd_putc(request_floor % 10 +'0');
				lcd_gotoxy(0,1);
				lcd_putc(current_floor / 10 +'0' );
				lcd_putc(current_floor % 10 +'0');

				_delay_ms(500);
			} 
			else 
			{
				state = DOOR;
			}
			
			break;
		case GOINGDOWN:
			// check if em button is pressed
			if (is_emergency_button_pressed())
			{
				state = EMERGENCY;
				break;
			}
			SPI_MasterTransmit(state);
			if (current_floor > request_floor)
			{
				current_floor--;
				lcd_clrscr();
				lcd_puts("Down");
				lcd_gotoxy(7, 0);
				lcd_puts("RF:");
				lcd_putc(request_floor / 10 +'0' );
				lcd_putc(request_floor % 10 +'0');
				lcd_gotoxy(0,1);
				lcd_putc(current_floor / 10 +'0' );
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
			lcd_puts("D opening");
			SPI_MasterTransmit(state);

			_delay_ms(5000);

			state = IDLE;
			break;
		
		case FAULT:
			lcd_clrscr();
			lcd_puts("fault");

			
			
			//blink movement led 3 times
			SPI_MasterTransmit(state);
			_delay_ms(4000);

			state= IDLE;
			
			
		break;
		
		case EMERGENCY:
			SPI_MasterTransmit(state);
			lcd_clrscr();
			lcd_puts("Emergency");

			
			
			// buzzer on -> plays melody 
			_delay_ms(8000);
			

			state = IDLE;
		break;
		
		}
		
	}
}