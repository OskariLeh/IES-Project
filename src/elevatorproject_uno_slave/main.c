/*
 * prjct.c
 *
 * Created: 21.4.2025 16.36.29
 * Author : oskul
 */ 
#define F_CPU 16000000UL
#define FOSC 16000000UL
#define BAUD 9600
#define MYUBR (FOSC/16/BAUD-1)

#include <avr/io.h>
#include <util/delay.h>
#include <util/setbaud.h>
#include <stdio.h>
#define IDLE 0
#define GOING_UP 1
#define GOING_DOWN 2
#define DOOR_OPEN 3
#define FAULT 4
#define EMERGENCY 5

void SPI_SlaveInit(void) {
	// Enable SPI
	SPCR |= (1 << SPE);
	
	// Set MISO as Output
	DDRB |= (1 << PB4);
}

int8_t SPI_ReceiveData(void) {
	
	// Wait for reception
	while(!(SPSR & (1<<SPIF))) {
		;	
	}
	
	return SPDR;
}

void USART_init(uint16_t ubrr) 
{
	UBRR0H = (unsigned char) (ubrr >> 8);
	UBRR0L = (unsigned char) ubrr;
	

	UCSR0B |= (1 << RXEN0) | (1 << TXEN0);
	UCSR0C |= (1 << USBS0) | (3 << UCSZ00);
}

void USART_Transmit(unsigned char data, FILE *stream)
{
	/* Wait until the transmit buffer is empty*/
	while(!(UCSR0A & (1 << UDRE0)))
	{
		;
	}
	
	UDR0 = data;
}

char USART_Receive(FILE *stream)
{
	/* Wait until the transmit buffer is empty*/
	while(!(UCSR0A & (1 << RXC0)))
	{
		;
	}
	
	return UDR0;
}

FILE uart_output = FDEV_SETUP_STREAM(USART_Transmit, NULL, _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, USART_Receive, _FDEV_SETUP_READ);

// Procedure for generating a square wave for the buzzer takes in frequency in Hz and duration in seconds
void buzz_tone(uint16_t frequency, uint16_t duration) {
	uint16_t T = 1000000 / frequency; // Time of period in micro seconds
	uint16_t count = (duration * 1000000) / T; // Number of periods for wanted duration
	  
	for (int i = 0; i<count; i++)
	{
		PORTD |= (1 << PD6);
		for (int j = 0; j < T/2; j ++)
		{
			_delay_us(1);
		}
		PORTD &= ~(1 << PD6);
		for (int j = 0; j < T/2; j ++)
		{
			_delay_us(1);
		}
	}
}

int main(void)
{	
	// Define Pins
	DDRB |= (1 << PB0); // Door Open LED as OUT
	DDRD |= (1 << PD7); // Movement LED as OUT
	DDRD |= (1 << PD6); // Buzzer as OUT
	
	USART_init(MYUBR);
	SPI_SlaveInit();
	
	stdout = &uart_output;
	stdin = &uart_input;
	
	int8_t state;
	
    while (1) 
    {
		state = SPI_ReceiveData();
		
		switch (state) {
			case IDLE:

				break;
			case DOOR_OPEN:
				PORTD &= ~(1 << PD7); // Turn off moving LED
			
				// Turn on DOOR_OPEN LED for 5s
				PORTB |= (1 << PB0);
				_delay_ms(5000);
				PORTB &= ~(1 << PB0);
				
				// -> IDLE
				break;
			case GOING_UP: //Fall through state
			case GOING_DOWN:
				PORTD |= (1 << PD7); // Turn on moving LED
				
				// -> DOOR_OPEN
				break;
			case EMERGENCY:
				// Blinks Movement LED 3 times
				for (int i = 0; i < 3; i++)
				{
					PORTD |= (1 << PD7);
					_delay_ms(500);
					PORTD &= ~(1 << PD7);
					_delay_ms(500);
				}
				
				// Turn on DOOR_OPEN LED
				PORTB |= (1 << PB0);
		
				// Melody plays
				buzz_tone(500, 1);
				buzz_tone(300, 1);
				buzz_tone(400, 1);
				buzz_tone(450, 1);
				
				// Door Close
				PORTB &= ~(1 << PB0);
				
				// -> IDLE
				break;
			case FAULT:
				// Blinks Movement LED 3 times
				for (int i = 0; i < 3; i++)
				{
					PORTD |= (1 << PD7);
					_delay_ms(500);
					PORTD &= ~(1 << PD7);
					_delay_ms(500);
				}
				
				//-> IDLE
				break;
			default:

				break;
		}
    }
}

