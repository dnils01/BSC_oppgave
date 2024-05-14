/*
 * CFile1.c
 *
 * Created: 15.01.2024 13:08:50
 *  Author: zalto
 */ 
#include "tcb.h"
#include "usart.h"
#include "dac.h"
#include "config.h"
#include "tca.h"


#include <avr/cpufunc.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


volatile uint32_t tcb0_period_us=0;
volatile uint32_t tcb1_period_us=0;
volatile uint32_t tcb0_last_time=0;
volatile uint32_t tcb1_last_time=0;
char TCB0_string[20];
char TCB1_string[20];
volatile uint8_t tcb0_new_reading=0;
volatile uint8_t tcb1_new_reading=0;


//   For measuring motor speed   //
void TCB0_init(void) {		
	TCB0.CNT = 0x0; // Ensure CNT register = 0
	
	TCB0.CTRLA = 1 << TCB_RUNSTDBY_bp
	| 0 << TCB_CASCADE_bp
	| 0 << TCB_SYNCUPD_bp
	|      TCB_CLKSEL_EVENT_gc	//Clock source 
	| 1 << TCB_ENABLE_bp ;	//Enable

	TCB0.CTRLB = (0 & TCB_CNTMODE_gm);   //Count mode
	TCB0.CCMP =AVG_FILTER_SIZE-1;	// Compare register value set
	TCB0.INTCTRL = TCB_CAPT_bm;	// Enable interrupt on capture.
	
	// Set TCB0 to be polled on event of PortB pin 5.
	EVSYS.CHANNEL0 = EVSYS_CHANNEL0_PORTB_PIN5_gc;  //   
	EVSYS.USERTCB0COUNT = 1; // Connect user to channel 0=n-1.
}

//   For measuring motor speed   //
void TCB1_init(void) {
	TCB1.CNT = 0x0;
	
	TCB1.CTRLA = 1 << TCB_RUNSTDBY_bp
	| 0 << TCB_CASCADE_bp
	| 0 << TCB_SYNCUPD_bp
	|      TCB_CLKSEL_EVENT_gc
	| 1 << TCB_ENABLE_bp ;
	
	TCB1.CTRLB = (0 & TCB_CNTMODE_gm);   //Count mode
	TCB1.CCMP =AVG_FILTER_SIZE-1;	// Compare register value set
	TCB1.INTCTRL = TCB_CAPT_bm;	// Enable interrupt on capture.
	
	// Set TCB0 to be polled on event of PortB pin 4.
	EVSYS.CHANNEL1 = EVSYS_CHANNEL1_PORTB_PIN4_gc;  //
	EVSYS.USERTCB1COUNT = 2; // connect to channel 0=n-1.
}


ISR(TCB0_INT_vect) {	// ISR for TCB0
	uint32_t tmp = read_tca_time();//Read tca value
	tcb0_period_us = tmp-tcb0_last_time;	//Calculate period
	tcb0_last_time=tmp;
	tcb0_new_reading=1;	//Raise flag for new reading available
	TCB0.INTFLAGS = TCB_CAPT_bm; // Clear interrupt flag	
}

ISR(TCB1_INT_vect) {	// ISR for TCB1
	uint32_t tmp = read_tca_time();//Read tca value
	tcb1_period_us = tmp-tcb1_last_time;	//Calculate period
	tcb1_last_time=tmp;
	tcb1_new_reading=1;	//Raise flag for new reading available
	TCB1.INTFLAGS = TCB_CAPT_bm; // Clear interrupt flag
}

uint32_t TCB0_read_period(void){	
	return tcb0_period_us;
}
uint32_t TCB1_read_period(void){
	return tcb1_period_us;
}

