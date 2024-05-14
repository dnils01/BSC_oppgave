/*
 * functions.c
 *
 * Created: 18.03.2024 22:55:51
 *  Author: zalto
 */ 

#include "adc.h"
#include "config.h"
#include "dac.h"
#include "display.h"
#include "i2c.h"
#include "functions.h"
#include "tca.h"
#include "usart.h"
#include "variables.h"

#include <avr/cpufunc.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

extern const address = 0x27;
uint8_t buttondown =0;
uint8_t buttonup = 0;
uint8_t buttonenter = 0;

uint8_t analog_in_status = 0;
uint8_t analog_out_status = 0;
uint8_t digital_in_status = 1;
uint8_t digital_out_status = 1;

volatile uint16_t dutycycle_value=0;
volatile uint16_t rpm_value=0;
volatile uint16_t backemf_value=0;
volatile uint16_t current_value=0;

void CLKCTRL_INTERNAL_16M_init(){
	ccp_write_io((void *)&(CLKCTRL.OSCHFCTRLA) ,CLKCTRL_FRQSEL_16M_gc         /* 16 MHz internal clock */
	| 0 << CLKCTRL_AUTOTUNE_bp /* Auto-Tune enable: disabled */
	| 0 << CLKCTRL_RUNSTDBY_bp /* Run standby: disabled */);
}

void LEDS_init(void){
	PORTF.DIRCLR = (1<<DIN_pin) | (1<<DOUT_pin) | (1<<AIN_pin) |(1<<AOUT_pin);	// Clear pins
	PORTF.DIRSET = (1<<DIN_pin) | (1<<DOUT_pin) | (1<<AIN_pin) |(1<<AOUT_pin);	// Set pins as output
	PORTF.OUTSET = (1<<DIN_pin)| (1<<DOUT_pin) | (1<<AIN_pin) |(1<<AOUT_pin);	// Set high
	
}

void button_init(void){
	PORTD.DIRCLR   = PIN5_bm | PIN6_bm | PIN7_bm;	// Clear pins
	PORTD.PIN5CTRL = PORT_ISC_FALLING_gc; //PORT_PULLUPEN_bm | 	//Enable pullup and interrupt on falling edge
	PORTD.PIN6CTRL = PORT_ISC_FALLING_gc;
	PORTD.PIN7CTRL = PORT_ISC_FALLING_gc;		
}


void button_handler(){
	if(buttondown==1){		
		ButtonDownPressed();
		buttondown=0;	//Reset state of button			
	}
	else if (buttonup==1){
		ButtonUpPressed();
		buttonup=0;	//Reset state of button
	}
	else if (buttonenter==1){
		ButtonEnterPressed();		
		buttonenter=0;	//Reset state of button
	}
}

void led_handler(void){
	if (digital_in_status==1){
			PORTF.OUT |=(1<<DIN_pin);	//enable led for DIN		
	}
	else if (digital_in_status==0){
		PORTF.OUT &=~(1<<DIN_pin);	//disable led for DIN		
	}
	if (digital_out_status==1){
		PORTF.OUT |=(1<<DOUT_pin);		//enable led for DOUT		
	}
	else if (digital_out_status==0){
		PORTF.OUT &=~(1<<DOUT_pin);	//disable led for DOUT		
	}	
	if (analog_in_status==1){
		PORTF.OUT |=(1<<AIN_pin);		//enable led for AIN
	}
	else if (analog_in_status==0){
		PORTF.OUT &=~(1<<AIN_pin);	//disable led for AIN
	}
	if (analog_out_status==1){
		PORTF.OUT |=(1<<AOUT_pin);		//enable led for AOUT
	}
	else if (analog_out_status==0){
		PORTF.OUT &=~(1<<AOUT_pin);	//disable led for AOUT
	}	
}

void GeneratorSetDutycycle(uint16_t duty){
	if (duty==0){
		//TCA0_PWM_disable();
		TCA0.SINGLE.CTRLB &= ~TCA_SINGLE_CMP2EN_bm;
		PORTA.OUTSET |=PIN2_bm;	
	}	
	else if(duty<=100&&duty>=1){
		uint16_t tmp_period=duty*(TCA0_period/100);
		TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP2EN_bm;
		TCA0_PWM_set_duty(200);
		
		uint16_t tmp = 400-tmp_period;
		TCA0_PWM_set_duty(tmp);			
	}
}

void message_handler_HMI(void){		//Function for handling messages and assigning values
	USART1buffer2message();
	if(USART1message_available){	//If message is available
		if (USART1message[0] == 'R'){	//RPM message
			char tmp[3];
			tmp[0]=USART1message[1];
			tmp[1]=USART1message[2];
			tmp[2]=USART1message[3];
			tmp[3]=USART1message[4];
			update_variable(&rpm_value, atoi(tmp));						
		}
		else if (USART1message[0] == 'I'){ //Current message
			char tmp[3];
			tmp[0]=USART1message[1];
			tmp[1]=USART1message[2];
			tmp[2]=USART1message[3];
			tmp[3]=USART1message[4];
			update_variable(&current_value, atoi(tmp));
		}
		else if (USART1message[0] == 'B'){ //BACK_emf message
			char tmp[3];
			tmp[0]=USART1message[1];
			tmp[1]=USART1message[2];
			tmp[2]=USART1message[3];
			tmp[3]=USART1message[4];
			update_variable(&backemf_value, atoi(tmp));
		}
		else if (USART1message[0] == 'D'){ //Motor duty cycle message
			char tmp[3];
			tmp[0]=USART1message[1];
			tmp[1]=USART1message[2];
			tmp[2]=USART1message[3];
			tmp[3]=USART1message[4];			
			update_variable(&dutycycle_value, atoi(tmp));
		}
		else if (USART1message[0] == 'G'){ //Generator duty cycle message
			char tmp[3];
			tmp[0]=USART1message[1];
			tmp[1]=USART1message[2];
			tmp[2]=USART1message[3];
			update_variable(&adjustgenerator, atoi(tmp));
			if (generatorADCenable==0)
			{
				if (adjustgenerator==0){		//Turn off generator
					update_variable_uint8(&generatorenable,0);
				}
				else if(adjustgenerator>=1 && adjustgenerator<=100){
					update_variable_uint8(&generatorenable,1);			//Turn on generator
					update_variable_uint8(&generatorADCenable,0);
				}
				else if (adjustgenerator>=101){
					update_variable(&adjustgenerator, 100);
				}
			}
		}
		else if (USART1message[0] == 'Y'){ //Setpoint message
			char tmp[3];
			tmp[0]=USART1message[1];
			tmp[1]=USART1message[2];
			tmp[2]=USART1message[3];
			tmp[3]=USART1message[4];
			update_variable(&adjustsetpoint, atoi(tmp));
			if(adjustsetpoint>=4000){
				update_variable(&adjustsetpoint, 4000);
			}
		}
		else if (USART1message[0] == 'p'){ //pid kp message
			char tmp[3];
			tmp[0]=USART1message[1];
			tmp[1]=USART1message[2];
			tmp[2]=USART1message[3];
			tmp[3]=USART1message[4];
			update_variable(&adjustkp, atoi(tmp));
			if(adjustkp<=0){
				update_variable(&adjustkp, 0);
			}
		}
		else if (USART1message[0] == 'i'){ //pid ki message
			char tmp[3];
			tmp[0]=USART1message[1];
			tmp[1]=USART1message[2];
			tmp[2]=USART1message[3];
			tmp[3]=USART1message[4];
			update_variable(&adjustki, atoi(tmp));
			if(adjustki<=0){
				update_variable(&adjustki, 0);
			}
		}
		else if (USART1message[0] == 'd'){ //pid kd message
			char tmp[3];
			tmp[0]=USART1message[1];
			tmp[1]=USART1message[2];
			tmp[2]=USART1message[3];
			tmp[3]=USART1message[4];
			update_variable(&adjustkd, atoi(tmp));
			if(adjustkd<=0){
				update_variable(&adjustkd, 0);
			}
		}
				
		strcpy(USART1message, "");
		update_variable_uint8(&USART1message_available,0);
	}
}

void SendStatus2otherMCU(void){		//function for sending status of everything to other main mcu
	if (io_changed==1){
		char buffer[5];
		io_changed=0;
		USART1_transmitt('<');
		USART1_transmitt('A');
		USART1_transmit_int(digital_in_status);	//IO status for index 0-3
		USART1_transmit_int(digital_out_status);
		USART1_transmit_int(analog_in_status);
		USART1_transmit_int(analog_out_status);
		USART1_transmitt('>');
	}
	
	if (regulatormode_changed==1){
		char buffer[5];
		regulatormode_changed=1;
		USART1_transmitt('<');
		USART1_transmitt('R');	//index 0
		USART1_transmit_int(regulatormodestatus);//status for enabled regulator index 1
		snprintf(buffer,5, "%04d", adjustsetpoint);
		USART1_transmitt_char_array(0,buffer, 4);	// setpoint for index 2-5
		strcpy(buffer, "");
		snprintf(buffer,5, "%04d", adjustkp);
		USART1_transmitt_char_array(0,buffer, 4);	// kp for index 6-9
		strcpy(buffer, "");
		snprintf(buffer,5, "%04d", adjustki);
		USART1_transmitt_char_array(0,buffer, 4);	// ki for index 10-13
		strcpy(buffer, "");
		snprintf(buffer,5, "%04d", adjustkd);
		USART1_transmitt_char_array(0,buffer, 4);	// kd for index 14-17
		USART1_transmitt('>');
	}
	
	if (generator_changed==1){
		char buffer[5];
		generator_changed=0;
		USART1_transmitt('<');
		USART1_transmitt('G');
		snprintf(buffer,4, "%03d", adjustgenerator);// generator duty cycle 
		USART1_transmitt_char_array(0,buffer, 3);
		USART1_transmitt('>');
	}
}

void update_variable(volatile uint16_t *var,uint16_t new_value){	
	*var=new_value;	
}
void update_variable_uint8(volatile uint8_t *var, uint8_t new_value){
	*var=new_value;
}

uint16_t int2percentage(uint16_t input, uint16_t input_max_value){
	float percentage = ((float)input * (100.0 / (float) input_max_value));
	uint16_t tmp_value=(uint16_t)percentage;
	return tmp_value;
}

