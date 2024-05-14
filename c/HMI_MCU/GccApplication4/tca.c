#include <avr/cpufunc.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "usart.h"
#include "tca.h"
#include "config.h"

/*   Relevante verdier for periode   */
//	3000 --> 8kHz



uint32_t counted_25ms=0;
uint32_t counted_time=0;
uint8_t time2updatescreen=0;
uint8_t time2sendData=0;

void TCA0_PWM_init(void){
	PORTA.DIRSET |= PIN2_bm;	// Port A pin 2 as output
	
	PORTMUX.TCAROUTEA = PORTMUX_TCA0_PORTA_gc;	//Route TCA to port A
	TCA0.SINGLE.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc ;		//Enable single-slope PWM mode
	
	TCA0.SINGLE.PER = TCA0_period;	//Set period for frq
	TCA0.SINGLE.CMP2 = TCA0_dutycycle;	// Set PWM duty cycle
	
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc;	//Clock division = 1
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;	//Enable TC0
}
void TCA0_PWM_enable(void){
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;				
}
void TCA0_PWM_disable(void){
	TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm;
}
void TCA0_PWM_set_duty(uint16_t dutycycle){	
	TCA0.SINGLE.CMP2 = dutycycle;	
}
void TCA0_PWM_set_period(uint16_t period){	
	TCA0.SINGLE.PER = period;
}
