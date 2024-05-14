/*
 * init.c
 *
 * Created: 18.03.2024 23:28:45
 *  Author: zalto
 */ 

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


#include "init.h"
#include "functions.h"
#include "tca.h"
#include "tcb.h"
#include "usart.h"
#include "variables.h"

void Initialize_system(void){
	CLKCTRL_INTERNAL_16M_init();
	ADC0_init();	
	_delay_ms(50);	
	DAC0_init();
	TCA0_PWM_init();
	TCA1_timer_init();
	TCB0_init();
	TCB1_init();
	USART0_init();
	USART3_init();	
	current_calibration();	
}