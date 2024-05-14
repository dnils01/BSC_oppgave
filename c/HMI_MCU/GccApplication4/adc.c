
//#define F_CPU 16000000ul
#include "adc.h"
#include "config.h"
#include "functions.h"
#include "usart.h"
#include "variables.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
volatile uint16_t AIN_selected;
volatile uint16_t AIN0_value=5000;
volatile uint16_t AIN1_value=5000;
volatile uint16_t AIN2_value=5000;
volatile uint16_t AIN3_value=5000;
volatile uint16_t AIN4_value=5000;
volatile uint8_t adc_conversion_ready =0;
volatile uint8_t ADC_reading_ready=0;


void ADC0_init(void) {	
	VREF.ADC0REF |= VREF_REFSEL_2V500_gc ;//Configure ADC voltageref
	ADC0.CTRLC |= ADC_PRESC_DIV256_gc ;  // 1MHz @ 16MHz main clock
	//ADC0_CTRLB |=ADC_SAMPNUM_ACC128_gc;	
	
	ADC0.CTRLA |= ADC_ENABLE_bm	;		//enable ADC
	ADC0.CTRLA |= ADC_RESSEL_12BIT_gc;	//12-bit resolution
	ADC0.CTRLA |= ADC_FREERUN_bm;		//freerun mode
	
	//ADC0.INTCTRL |= ADC_RESRDY_bm;		//Enable interrupt when result ready
	
	ADC0_change_input(1);//set input to PD1
}
	
void ADC0_start(void){
	ADC0.COMMAND = ADC_STCONV_bm; //starting conversion
}
void ADC0_stop(void){
	ADC0.COMMAND = ADC_SPCONV_bm; //stopping conversion
}

uint16_t ADC0_read_value(uint8_t channel){	
	if (AIN_selected!=channel){
		ADC0_change_input(channel);	
	}
	
	while (!(ADC0.INTFLAGS&ADC_RESRDY_bm));
	return ADC0.RES;
}

uint16_t ADC0_read_assigned_value(uint8_t channel){
	ADC0_stop();
	uint16_t var = ADC0.RES;
	ADC0_change_input(channel);	
	while (!(adc_conversion_ready==1));
	if(channel==0){
		return AIN0_value;
	}
	else if(channel==1){
		return AIN1_value;
	}
	else if(channel==2){
		return AIN2_value;
	}
	else if(channel==3){
		return AIN3_value;
	}
	else if(channel==4){
		return AIN4_value;
	}
	adc_conversion_ready =0;
}
void ADC0_assign_value(void){
	adc_conversion_ready =1;
	if(AIN_selected==0){
		AIN0_value= ADC0.RES;
	}
	else if(AIN_selected==1){
		AIN1_value= ADC0.RES;
	}
	else if(AIN_selected==2){
		AIN2_value= ADC0.RES;
	}
	else if(AIN_selected==3){
		AIN3_value= ADC0.RES;
	}	
	else if(AIN_selected==4){
		AIN4_value= ADC0.RES;
	}
	
	
}

void ADC0_change_input(uint8_t input){
	//adc0_stop();
	AIN_selected = input;
	if (input == 0 ){
		ADC0.MUXPOS = ADC_MUXPOS_AIN0_gc;		
	}
	else if (input == 1){
		ADC0.MUXPOS = ADC_MUXPOS_AIN1_gc;
	}
	else if (input == 2){
		ADC0.MUXPOS = ADC_MUXPOS_AIN2_gc;		
	}
	else if (input == 3){
		ADC0.MUXPOS = ADC_MUXPOS_AIN3_gc;		
	}
	else if (input == 4){
		ADC0.MUXPOS = ADC_MUXPOS_AIN4_gc;
	}	
	ADC0_start();
}

/*
ISR(ADC0_RESRDY_vect){
	uint16_t tmp=ADC0.RES;		
	float percentage = ((float)tmp * (2.5 / 4095.0) / 2.5) * 100.0;
	uint16_t tmp_value=(uint16_t)percentage;
	update_variable(adjustgenerator,tmp_value);	
}*/
	