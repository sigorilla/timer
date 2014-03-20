/**
 * Author: 		Stepanov Igor
 * Group:		213
 * Title:		Timer
 * Description:	Using two button for manage timer.
 * 				First button increase counter, second starts timer.
 * Target:		AtMega16A
 * Compiler: 	Atmel Studio 6
 * Date: 		04.03.2014
 */

#include <avr/io.h>									// Main header
#include <avr/interrupt.h>							// Need for working interrupts

#ifndef F_CPU
#define F_CPU 32768UL
#endif
#define __DELAY_BACKWARD_COMPATIBLE__
#include <util/delay.h>

uint8_t anods[] =									// Array for anod's registors	
{
	0b00000111,										// all off
	0b00000110,										// 1 - on
	0b00000101,										// 2 - on
	0b00000011										// 3 - on
};

uint8_t segment[] =									// Array for show digit on indicator
{
	0b00111111,    									// 0
	0b00000110,     								// 1
	0b01011011,     								// 2
	0b01001111,     								// 3
	0b01100110,     								// 4
	0b01101101,     								// 5
	0b01111101,     								// 6
	0b00000111,     								// 7
	0b01111111,     								// 8
	0b01101111,     								// 9
	0b00000000										// NO
};

#define DOT		0b10000000
#define NO		0b00000000
#define MINUS	0b01000000

volatile uint8_t tot_overflow;						// Counter for each overflow of Timer0
int Flag = 0;										// Flag shows work or not timer
int count = 0;										// Main counter

/*
 * Description of functions
 */
void ioinit(void);
void SwitchOn(int num);
ISR(INT1_vect);
ISR(INT0_vect);
ISR(TIMER0_OVF_vect);

/*
 * Main function of program
 */
int main(void) 
{	 
	ioinit(); 										// Initialization and settings
	while(1)
	{			
		if (Flag == 0) 
		{
			DDRD = 0b01110011;						// Enable buttons: 2 and 3 registor is log.1
 			PORTD |= _BV(PD1);						// Green LED is ON
			PORTD &= ~_BV(PD0);						// Red LED is OFF
 			SwitchOn(count);						// Display counter on indicator
		}
		else
		{
			DDRD = 0b01111111;						// Disable buttons 
			PORTD &= ~_BV(PD1);						// Green LED is OFF
			PORTD |= _BV(PD0);						// Red LED is ON
			while (tot_overflow < 255) 				// If it's going 1 second,
				SwitchOn(count);					// then update indicator
			TCNT0 = 0;								// Reset counter
			tot_overflow = 0;						// Reset overflow counter
			if (count >= 0) count--;				// Decrease counter
			else if (count < 0) Flag = 0;			// If counter negative, then that's all
		}
	}
	return 0;
} 

/*
 * Function of initialization
 */
void ioinit(void)
{
	cli();											// Disable all interrupts
	DDRD = 0b01110011;								// Settings for Port D
	PORTD = 0b00000011;										
	GICR = (1 << INT0) | (1 << INT1);				// Available interrupt for Int0/1
	MCUCR = (1 << ISC11) | (1 << ISC01); 			// Interrupts works when signal is high on Int0/1
	DDRA = 0b11111111;								// All ports A for Output
	PORTA = 0b00000000;								
	TCCR0 |= (1 << CS02);							// Prescale is 256
	TCNT0 = 0;										// Starting 
	TIMSK |= (1 << TOIE0);							// Available overflow interrupt of Timer0 
	tot_overflow = 0;								// Counter of overflow is 0
	count = 0;										// Main counter is 0
	sei();											// Enable all interrupts
}

/*
 * Function for show counter on indicator
 * It's working on dynamic indication
 */
void SwitchOn(int num)
{
	int digit = 0;
	for (int i = 1; i <= 3; i++)					// Loop for 3 anods on indicator
	{
		switch (i)									// Each anods must get log.0
		{											// then this digit becomes ON
			case 1: 
				PORTD &= ~_BV(PD4); 
				break;
			case 2: 
				PORTD &= ~_BV(PD5); 
				break;
			case 3: 
				PORTD &= ~_BV(PD6); 
				break;
			default: 
				break;
		}
		
		PORTA = NO;									// Reset digit on indicator
		if (num==-1) PORTA = MINUS;					// If counter is negative, then that's all
		else
		{
			switch(i)								// Counter decompose into components
			{
				case 1: 
					digit = num % 10; 
					break;
				case 2: 
					digit = num / 10 - (num / 100) * 10; 
					break;
				case 3: 
					digit = num / 100; 
					break;
				default: 
					digit = 0; 
					break;
			}
			PORTA = segment[digit];					
		}
		_delay_ms(5);
		PORTD |= _BV(PD4) | _BV(PD5) | _BV(PD6);	// log.1 for anods
	}	
	PORTA = NO;	
}

/*
 * Interrupt of INT0 (Port D2)
 * Button press
 */
ISR(INT0_vect)
{
	Flag = 1;										// Start timer
	count--;
}

/*
 * Interrupt of INT1 (Port D3)
 * Button press
 */
ISR(INT1_vect)
{
	if (count<1000) count++;						// Increase counter before it less then 1000
	else count = 0;									// Else reset counter
}

/*
 * Interrupt of overflow Timer0
 */
ISR(TIMER0_OVF_vect)
{
	tot_overflow++;									// Increase overflow counter when main counter
}													// of Timer0 is overflow