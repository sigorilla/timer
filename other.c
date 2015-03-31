/*
 * Shift Registor
 */

// Main header
#include <avr/io.h>
// Need for working interrupts
#include <avr/interrupt.h>

#define __DELAY_BACKWARD_COMPATIBLE__
#include <util/delay.h>

#define DS_PORT PORTB
#define DS_PIN PB0
#define ST_CP_PORT PORTB 
#define ST_CP_PIN PB1 
#define SH_CP_PORT PORTB 
#define SH_CP_PIN PB2 

#define DS_low() DS_PORT &= ~_BV(DS_PIN) 
#define DS_high() DS_PORT |= _BV(DS_PIN) 
#define ST_CP_low() ST_CP_PORT &= ~_BV(ST_CP_PIN) 
#define ST_CP_high() ST_CP_PORT |= _BV(ST_CP_PIN) 
#define SH_CP_low() SH_CP_PORT &= ~_BV(SH_CP_PIN) 
#define SH_CP_high() SH_CP_PORT |= _BV(SH_CP_PIN) 

int flag;

//Define of functions
void ioinit( void ); 
void output_led_state( unsigned int ); 

// Interrupt for Button
ISR( INT0_vect );	

int main( void ) { 

	ioinit(); 

	while (1) {	
		sei();
		if ( flag == 1 ) {
			for ( int i = 0; i < 8; i++ ) {
				while ( flag == 0 ) {
					sei();
					continue;
				}
				output_led_state( _BV(i) );
				_delay_ms( 100 );
			}

			for ( int i = 7; i >= 0; i-- ) {
				while ( flag == 0 ) {
					sei(); 
					continue;
				}	
				output_led_state( _BV(i) ); 
				_delay_ms( 100 ); 
			} 
		} 
	}
	return 0;
}

void ioinit( void ) {
	cli();

	DDRB 	= 0b00001111;
	PORTB 	= 0x00;

	// Setting for Int0 or press button
	// D2 for OUTPUT => 0
	DDRD = 0x00;
	PORTD = 0xff;
	// Enable interrupt for Int0
	GICR = 1 << INT0;
	// Interrupts works when signal is high on Int0
	MCUCR = 1 << ISC01;
	flag = 1;

	sei();
}

void output_led_state( unsigned int __led_state ) { 
	SH_CP_low(); 
	ST_CP_low(); 
	for ( int i = 0; i < 8; i++ ) { 
		if ( bit_is_set( __led_state, i ) ) {
			DS_high(); 
		} else { 
			DS_low();
		}
		SH_CP_high(); 
		SH_CP_low(); 
	} 
	ST_CP_high(); 
} 

/*
 * Interrupt of INT0 (Port D2)
 * Button press
 */
ISR( INT0_vect ) {
	PORTB ^= _BV( PB3 );
	flag = ( flag == 0 ) ? 1 : 0;
}
