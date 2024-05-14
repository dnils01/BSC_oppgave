/*
 * dac.c
 *
 * Created: 22.01.2024 13:07:52
 *  Author: zalto
 */ 

#include "config.h"
#include "dac.h"
#include "usart.h"
#include <string.h>
#define LSB_MASK                (0x03)

#include <avr/io.h>
#include <util/delay.h>


/*VDACREF = (DACREF/256) × VREF*/
void DAC0_init(void){
	VREF.DAC0REF = VREF_REFSEL_2V500_gc /* Select the VDD Voltage Reference for DAC */
	| VREF_ALWAYSON_bm;

	DAC0_enable();
	_delay_us(50);
	#ifdef debugmessages
	USART3_transmitt_char_array(0, "DAC initialized\n", strlen("DAC initialized\n"));
	#endif
	
}

void DAC0_setvalue(uint16_t value){	//value [0-1023]	 
	 DAC0.DATAL = (value & LSB_MASK) << 6;	// Set the two LSbs in DAC0.DATAL	 
	 DAC0.DATAH = value >> 2;	// Set the eight MSbs in DAC0.DATAH 
	 #ifdef debugmessages
	 USART3_transmitt_char_array(0, "DAC value set to ", strlen("DAC value set to "));
	 USART3_transmit_int(value);
	 USART3_transmitt('\n');
	 #endif
}

void DAC0_enable(void){
	DAC0.CTRLA = DAC_ENABLE_bm|DAC_OUTEN_bm;
	#ifdef debugmessages
	USART3_transmitt_char_array(0, "DAC enabled\n", strlen("DAC enabled\n"));
	#endif
}

void DAC0_disable(void){
	DAC0.CTRLA &= ~(DAC_ENABLE_bm | DAC_OUTEN_bm);
	USART3_transmitt_char_array(0, "DAC disabled\n", strlen("DAC disabled\n"));
}