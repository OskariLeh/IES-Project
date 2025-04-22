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




/*static void USART_init(uint16_t ubrr) 
{
	
	UBRR0H = (unsigned char) (ubrr >> 8); 
	UBRR0L = (unsigned char) ubrr; 
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0); 	
	UCSR0C |= (1 << USBS0) | (3 << UCSZ00);
	
}*/

void SPI_MasterInit(void) 
{
	/* Set MOSI and SCK output, all others input */ 
	DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2);
	/* Enable SPI, Master, set clock rate fck/16 */ 
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0); 
}


// datasheet p.193
void SPI_MasterTransmit(char cData) 
{
	PORTB &= ~(1 << PB0);
	_delay_ms(2);
	/* Start transmission */ 
	SPDR = cData; 
	/* Wait for transmission complete */ 
	while(!(SPSR & (1<<SPIF)))
	;
	PORTB |= (1 << PB0);
	_delay_ms(2);
}

/*static char USART_Receive(FILE *stream) 
{
	while(!(UCSR0A & (1 << RXC0)))
	{
		;
	}	
	return UDR0;
}*/

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
	return (PINA & (1 << PA0));		
}



int main(void)
{
    // initial lcd display and spi
	LCD_setup();
	//USART_init(MYUBRR);
	SPI_MasterInit();
	/*
	For SPI communication
	SS, MOSI and SCK
	*/
	//DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2);
	//Enable SPI, master, clock rate 
	//SPCR |= (1 << SPE) | (1 << MSTR) | (1 << SPR0);
	
	// elevator floor variables 
	static int8_t request_floor = 0 , current_floor = 0;

	
	
	//emergency button
	DDRA &= ~(1 << PA0);
	//PORTA |= (1 << PA0);
	    
    while (1) 
    {

 		
		PORTB &= ~(1 << PB0);
		switch (state)
		{
		case IDLE:
			
			lcd_clrscr();
			lcd_puts("Choose floor");
			lcd_gotoxy(0, 1);
			SPI_MasterTransmit('I');
			//uint8_t digits[2] = {0xFF, 0xFF};
			uint8_t key_signal_1,key_signal_2 ;
			//char key_signal;
			//int8_t floor_input  ;
			////key_signal = KEYPAD_GetKey();

			do {key_signal_1 = KEYPAD_GetKey();} while(!(key_signal_1 >= '0' && key_signal_1 <= '9'));
			do {key_signal_2 = KEYPAD_GetKey();} while(!(key_signal_2 >= '0' && key_signal_2 <= '9'));
			
			
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

			if (is_emergency_button_pressed())
			{
				state = EMERGENCY;
				break;
			}		
			SPI_MasterTransmit('U');
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
			
			break;
		case GOINGDOWN:
			if (is_emergency_button_pressed())
			{
				state = EMERGENCY;
				break;
			}
			SPI_MasterTransmit('D');
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
			lcd_puts("D opening");
			SPI_MasterTransmit('O');
			_delay_ms(5000);
			state = IDLE;
		
		// door open led to uno (5 sec)
		// door open 
		// back to idle state = IDLE
		
			
			break;
		
		case FAULT:
			lcd_puts("fault");
			
			
			//blink movement led 3 times
			SPI_MasterTransmit('F');
			_delay_ms(4000);
			state= IDLE;
			
			
		break;
		
		case EMERGENCY:
			SPI_MasterTransmit('E');
			lcd_clrscr();
			lcd_puts("Emergency");
			
			
			// buzzer on -> plays melody 
			_delay_ms(4000);
			
			state = IDLE;
		break;
		
		
		}
		
	
	
	}
}

