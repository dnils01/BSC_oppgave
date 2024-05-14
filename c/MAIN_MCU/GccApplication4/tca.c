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
#include "variables.h"
/*   Relevante verdier for periode   */
//	3000 --> 8kHz


#define TCA0_dutycycle 200;
#define TCA1_period  400;
#define TCA1_dutycycle 100;
volatile uint32_t counted_10ms=0;
volatile uint32_t counted_time=0;


void TCA1_timer_init(void){	//Timer for counting us with overflow @50ms
	
	TCA1.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV16_gc;
	TCA1.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc;
	TCA1.SINGLE.EVCTRL &= ~(TCA_SINGLE_CNTAEI_bm);
	TCA1.SINGLE.PER = 9999;
	TCA1.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
	
	TCA1.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
}



void TCA0_PWM_init(void){
	PORTA.DIR |= PIN2_bm;	// Port A pin 2 as output
	PORTE.DIR |= PIN0_bm;	
	PORTMUX.TCAROUTEA = PORTMUX_TCA0_PORTA_gc;	//Route TCA to port A
	TCA0.SINGLE.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc ;		//Enable single-slope PWM mode	
	TCA0.SINGLE.PER = TCA0_period;	//Set period for frq
	TCA0.SINGLE.CMP0 = TCA0_dutycycle;
	TCA0.SINGLE.CMP2 = TCA0_dutycycle;	// Set PWM duty cycle	
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc;	//Clock division = 1
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;	//Enable TC0
}

void TCA0_change_output(uint8_t output){		// Change output for PWM signal
	/*
	if (output==0){		
		TCA0.SINGLE.CTRLB &= !TCA_SINGLE_CMP0EN_bm;	//disable cmp0 channel(PE0)
		//PORTA.DIRSET |=PIN2_bm;	//set PA2 as output
		PORTE.OUTCLR |=PIN0_bm;	//set PE0 HIGH
		PORTE.DIR |=PIN2_bm;
		
		PORTMUX.TCAROUTEA = PORTMUX_TCA0_PORTA_gc;	//Route PWM to PORT A
		TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP2EN_bm;	//Enable cmp2 channel(PA2)
	}
	else if (output==0){
		//PORTE.OUT &=!PIN0_bm;
		//PORTE.DIR |=PIN0_bm;
		TCA0.SINGLE.CTRLB &= !TCA_SINGLE_CMP2EN_bm;	//disable cmp2 channel(PA2)
		PORTA.OUT &= !PIN2_bm; //set PA2 LOW
		
		PORTMUX.TCAROUTEA = PORTMUX_TCA0_PORTE_gc;	//Route PWM to PORT E
		
		TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP0EN_bm;	//Enable cmp0 channel(PE0)
	}*/
}

void TCA0_PWM_enable(void){
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
}

void TCA0_PWM_disable(void){
	TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm;
}

void TCA0_PWM_set_duty(uint16_t input){
	if (slowdecay_selected==0){
		TCA0.SINGLE.CMP2 = input;
	}
	else if(slowdecay_selected==1){
		TCA0.SINGLE.CMP0 = input;
	}
}

void TCA0_PWM_set_period(uint16_t period){	
	TCA0.SINGLE.PER = period;	
}

void TCA1_PWM_init(void){
	PORTC.DIRSET |= PIN6_bm;						//Set pc6 as output
	PORTMUX.TCAROUTEA = PORTMUX_TCA1_PORTE_gc;		//Set output to port c
	TCA1.SINGLE.CTRLB = TCA_SINGLE_CMP2EN_bm		//Enable compare channel 2
	|TCA_SINGLE_WGMODE_SINGLESLOPE_gc ;				//Select singleslope wavegeneration	   
	TCA1.SINGLE.PER = TCA1_period;							// PWM period, counted in clockcycles
	TCA1.SINGLE.CMP2 = TCA1_dutycycle;						// PWM duty cycle period, counted in clockcycles	   
	TCA1.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc;	//Set clk_div to 1,
}

void TCA1_PWM_enable(void){
	TCA1.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;	
}
void TCA1_PWM_disable(void){
	TCA1.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm;
}
void TCA1_PWM_set_duty(uint16_t dutycycle){
	TCA1.SINGLE.CMP2 = dutycycle;
}
void TCA1_PWM_set_period(uint16_t period){
	TCA1.SINGLE.PER = period;
}


/*
ISR(TCA0_OVF_vect) {

	//seconds_from_start= seconds_from_start+1;


	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
	//printf("%d\n", seconds_from_start);
	return;
}
*/

ISR(TCA1_OVF_vect){
	counted_10ms++;	
	TCA1.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}

uint32_t read_tca_time(void){	
	counted_time = TCA1.SINGLE.CNT+(10000*counted_10ms);	//Calculating time in us.
	return counted_time;	
}

void reset_tca_time(void){
	TCA1.SINGLE.CNT=0;
	counted_10ms=0;
}