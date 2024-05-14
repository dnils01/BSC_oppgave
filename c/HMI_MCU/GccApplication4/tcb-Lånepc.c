/*
 * CFile1.c
 *
 * Created: 15.01.2024 13:08:50
 *  Author: zalto
 */ 
#include "tcb.h"
#include "usart.h"
#include "dac.h"

#define F_CPU 4000000ul

#include <avr/cpufunc.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

uint32_t time_us=0;
uint32_t time_s=0;
uint32_t last_time_us=0;
uint32_t last_time_s=0;
uint32_t period_us=0;

volatile uint16_t time_lowerPart;
volatile uint16_t time_upperPart;



void TCB0_init() {
	TCB0.CNT = 0x0; /* Count: 0x0 */
	
	TCB0.CTRLA = 0 << TCB_RUNSTDBY_bp
	| 0 << TCB_CASCADE_bp
	| 0 << TCB_SYNCUPD_bp
	|      TCB_CLKSEL_EVENT_gc
	| 1 << TCB_ENABLE_bp ;

	TCB0.CTRLB = (0 & TCB_CNTMODE_gm);   //Count mode
	TCB0.EVCTRL = 0;
	
	TCB0.CCMP = 8;
	TCB0.INTCTRL = TCB_CAPT_bm;
	// Set TCB0 to be polled on event of PortB pin 5.
	EVSYS.CHANNEL0 = EVSYS_CHANNEL0_PORTB_PIN5_gc;  //   
	EVSYS.USERTCB0COUNT = 1; // connect to channel 0=n-1.
	printf("TCB0 initialized\n");
}


void TCB1_init() {
	TCB1.CNT = 0x0; /* Count: 0x0 */
	
	TCB1.CTRLA = 0 << TCB_RUNSTDBY_bp
	| 0 << TCB_CASCADE_bp
	| 0 << TCB_SYNCUPD_bp
	|      TCB_CLKSEL_EVENT_gc
	| 1 << TCB_ENABLE_bp ;

	TCB1.CTRLB = TCB_CNTMODE_FRQ_gc;//(0 & TCB_CNTMODE_gm);   //Count mode
	TCB1.EVCTRL = 0;
	
	// Set TCB0 to be polled on event of PortB pin 4.
	EVSYS.CHANNEL1 = EVSYS_CHANNEL1_PORTB_PIN4_gc;  //
	EVSYS.USERTCB1COUNT = 2; // connect to channel 0=n-1.
	printf("TCB1 initialized\n");
}
void TCB2_init() {
	
	    // Set clock source to CLK_PER, and set the desired prescaler
	   TCB2.CTRLA = TCB_CLKSEL_DIV2_gc     /* CLK_PER */
	   | 1 << TCB_ENABLE_bp   /* Enable: enabled */
	   | 0 << TCB_RUNSTDBY_bp /* Run Standby: disabled */
	   | 0 << TCB_SYNCUPD_bp  /* Synchronize Update: disabled */
	   | 0 << TCB_CASCADE_bp; /* Cascade Two Timer/Counters: disabled */

	    // Set the period for the desired overflow time
	    TCB2.CCMP = 120; 

	    // Enable the TCB in periodic mode
	    //TCB2.CTRLB = TCB_CNTMODE_CAPT_gc;

	
		
		TCB2.INTCTRL = 1 << TCB_CAPT_bp   /* Capture or Timeout: enabled */
		| 0 << TCB_OVF_bp; /* OverFlow Interrupt: disabled */
		printf("TCB2 initialized\n");
		
		
}
void TCB3_init() {
	
	// Set clock source to CLK_PER, and set the desired prescaler
	TCB3.CTRLA = TCB_CLKSEL_DIV1_gc     /* CLK_PER */
	| 1 << TCB_ENABLE_bp   /* Enable: enabled */
	| 0 << TCB_RUNSTDBY_bp /* Run Standby: disabled */
	| 0 << TCB_SYNCUPD_bp  /* Synchronize Update: disabled */
	| 0 << TCB_CASCADE_bp; /* Cascade Two Timer/Counters: disabled */

	// Set the period for the desired overflow time
	TCB3.CCMP = 4000;

	// Enable the TCB in periodic mode
	//TCB2.CTRLB = TCB_CNTMODE_CAPT_gc;

	
	
	TCB3.INTCTRL = 1 << TCB_CAPT_bp   /* Capture or Timeout: enabled */
	| 0 << TCB_OVF_bp; /* OverFlow Interrupt: disabled */
	printf("TCB2 initialized\n");
	
	
}
void TCB2TCB3_Cascade_init() {
	 TCB2.CTRLA = TCB_CLKSEL_DIV1_gc     /* CLK_PER */
	 | 1 << TCB_ENABLE_bp   /* Enable: enabled */
	 | 0 << TCB_RUNSTDBY_bp /* Run Standby: disabled */
	 | 0 << TCB_SYNCUPD_bp  /* Synchronize Update: disabled */
	 | 0 << TCB_CASCADE_bp; /* Cascade Two Timer/Counters: disabled */
	EVSYS.CHANNEL2 = 0xA5;  //
	EVSYS.USERTCB3COUNT = 3; // connect to channel 2=n-1.
	TCB2.INTCTRL = 1 << TCB_CAPT_bp   /* Capture or Timeout: enabled */
	| 0 << TCB_OVF_bp; /* OverFlow Interrupt: disabled */
	// Set clock source to CLK_PER, and set the desired prescaler
	TCB3.CTRLA = 0 << TCB_RUNSTDBY_bp
	| 1 << TCB_CASCADE_bp
	| 0 << TCB_SYNCUPD_bp
	|      TCB_CLKSEL_EVENT_gc
	| 1 << TCB_ENABLE_bp ;

	// Set the period for the desired overflow time
	TCB3.CCMP = 20000;

	// Enable the TCB in periodic mode
	TCB3.CTRLB = (0 & TCB_CNTMODE_gm);

	// Enable the TCB
	TCB3.CTRLA |= TCB_ENABLE_bm;
	
	//TCB2.INTCTRL = TCB_CAPT_bm;
	printf("TCB3 initialized\n");
	
}

ISR(TCB0_INT_vect) {
	// Your code to be executed on each TCB overflow
	period_us=5*((uint32_t)time_us-(uint32_t)last_time_us);
	time_lowerPart = (uint16_t)(period_us & 0xFFFF); // Mask the lower 16 bits
	time_upperPart = (uint16_t)((period_us >> 16) & 0xFFFF); // Shift right by 16 and mask the next 16 bits
	printf("<%u%u]", upperPart,lowerPart);
	//printf("<%u,%u,%u\n", time_s, time_100us,period);
	TCB0.INTFLAGS = TCB_CAPT_bm; // Clear the interrupt flag 
	last_time_us=time_us;
}
ISR(TCB2_INT_vect){
	time_us=time_us+10;
	if (time_us==1000000){
		PORTB.OUTTGL = PIN3_bm;
			
	}
	
	TCB2.INTFLAGS = TCB_CAPT_bm;
}