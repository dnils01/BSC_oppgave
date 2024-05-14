#define F_CPU 16000000ul

#include <avr/cpufunc.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h> 

#include "adc.h"
#include "config.h"
#include "dac.h"
#include "display.h"
#include "i2c.h"
#include "init.h"
#include "functions.h"	
#include "tca.h"
#include "usart.h"
#include "variables.h"
		
uint8_t Menu_handler_counter=0;

int main(void)
{	Initialize_system();
	_delay_ms(50);
	//USART3_transmitt_char_array(0, "All systems go\n", strlen("All systems go\n"));
	sei();	
	adjustgenerator=0;
	
	while(1){	
		led_handler();		//set status leds
		button_handler();	//read if buttons have been pressed
		message_handler_HMI();	//Handle messsages from other mcu if received.
		
		if (Menu_handler_counter==60){	//Make it so menu handler don't run too often
			Menu_handler_counter=0;
			Menu_handler();	//handles menu
		}
		Menu_handler_counter++;	
		
		if(generatorADCenable==1 && generatorenable){	//if ADC control mode
			ADC0_start();
			uint16_t tmp = int2percentage(ADC0_read_value(1),4095);
			update_variable(&adjustgenerator, tmp);
			GeneratorSetDutycycle(adjustgenerator);		
						
		}
		if(adjustgenerator_old != adjustgenerator && generatorenable){ //if variable changed
			GeneratorSetDutycycle(adjustgenerator);
		}		
		update_variable(&adjustgenerator_old, adjustgenerator);
		
		if (time2sendData==1){		//enough time has passed to send update to main mcu
			SendStatus2otherMCU();
			time2sendData=0;
		}
			
			
    }
}

ISR(PORTD_PORT_vect){	// Button interrupt service routine
	// Check which pin triggered the interrupt	
	if (PORTD.INTFLAGS & PIN5_bm) {
		// Clear the interrupt flag
		buttondown = 1;
		PORTD.INTFLAGS = PIN5_bm;		
	}
	else if (PORTD.INTFLAGS & PIN6_bm) {
		// Clear the interrupt flag
		buttonenter=1;		
		PORTD.INTFLAGS = PIN6_bm;		
	}
	else if (PORTD.INTFLAGS & PIN7_bm) {
		// Clear the interrupt flag
		buttonup = 1;
		PORTD.INTFLAGS = PIN7_bm;		
	}	
}

