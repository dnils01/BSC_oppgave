/*
 * tcb.c
 *
 * Created: 09.05.2024 11:31:34
 *  Author: zalto
 */ 


#include <avr/interrupt.h>
#include <stdint.h>
#include "tcb.h"
#include "variables.h"
volatile uint32_t time_counter;

void TCB0_init(void){
	
	
	TCB0.CTRLA = TCB_CLKSEL_DIV2_gc     // CLK_PER
	| 1 << TCB_ENABLE_bp   // Enable: enabled
	| 0 << TCB_RUNSTDBY_bp // Run Standby: disabled
	| 0 << TCB_SYNCUPD_bp  // Synchronize Update: disabled
	| 0 << TCB_CASCADE_bp; // Cascade Two Timer/Counters: disabled

	// Set the period for the desired overflow time
	TCB0.CCMP = 29999;
	
	TCB0.INTCTRL = 1 << TCB_CAPT_bp   // Capture or Timeout: enabled
	| 0 << TCB_OVF_bp; // OverFlow Interrupt: disabled
}


ISR(TCB0_INT_vect) {	// ISR for TCB0
	time_counter++;
	if (time_counter==10){
		time_counter=0;
		time2sendData=1;
	}
	
	TCB0.INTFLAGS = TCB_CAPT_bm; // Clear interrupt flag
}