/*
 * usart.c
 */


#include "usart.h"
#include "config.h"


#include <avr/cpufunc.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


volatile char USART1transmitt_buffer[50];
volatile char USART1RX_buffer[50];
volatile char USART1TX_buffer[50];
volatile uint8_t USART1RX_buffer_write=0;
volatile uint8_t USART1RX_buffer_read=0;
volatile uint8_t USART1TX_buffer_write=0;
volatile uint8_t USART1TX_buffer_read=0;
volatile uint8_t USART1next = 0;
volatile uint8_t USART1last = 0;
volatile uint8_t USART1message_available = 0;
volatile uint8_t USART1message_ready=0;
uint8_t USART1TX_buffer_size=50;




volatile uint8_t marker_pos = 0;


void USART1_init(){
	USART1.BAUD = 556/2;		
	USART1.CTRLB |= USART_RXEN_bm | USART_TXEN_bm | USART_RXMODE_CLK2X_gc;//USART_RXMODE_CLK2X_gc; //enable RX TX
	PORTC.DIR |= (1<<0);	// PC0 as output
	//PORTB.PINCONFIG |= PORT_PULLUPEN_bm;
	PORTC.PINCTRLSET |= (1<<1); //PC1 as input	
	USART1.CTRLA |= USART_RXCIE_bm; //RX interrupt enable	
	USART1.CTRLC = USART_SBMODE_2BIT_gc; //Double stop bit
	/* interrupt on TX */
	USART1.CTRLA |= USART_TXCIE_bm;
	USART1.CTRLC = USART_CHSIZE_8BIT_gc;
	
	#ifdef debugmessages
	USART3_transmitt_char_array(0, "USART1 initialized\n", strlen("USART1 initialized\n"));
	#endif
}


void USART1_transmitt(char c){
	USART1TX_buffer[USART1TX_buffer_write] = c;	//Write char to buffer.
	USART1TX_buffer_write++; //Increment write position
	if (USART1TX_buffer_write == (USART1TX_buffer_size+1)){	//Reset position at end of array
		USART1TX_buffer_write=0;
	}
	
	if((USART1.STATUS & USART_DREIF_bm)){	//If empty data register, start the transmitting
		USART1.TXDATAL=USART1TX_buffer[USART1TX_buffer_read];	//Transmitt the next char
		USART1TX_buffer_read++;	//Increment read position
		if (USART1TX_buffer_read ==(USART1TX_buffer_size+1)){	//Reset position at end of array
			USART1TX_buffer_read=0;
		}
	}
	_delay_us(18);  //Delay because of problems with transmitting strings, if not there only 3 first chars transmitted.
}



/* transmitting*/
void USART1_transmitt_string(char string[]){
	for(uint8_t i = 0; i<strlen(string); i++){
		USART1_transmitt(string[i]);
	}	
}

void USART1_transmitt_char_array(uint8_t addEndLine,char string[], uint8_t length){
	for(uint8_t i = 0; i<length; i++){
		USART1_transmitt(string[i]);
	}
	if(addEndLine){
		USART1_transmitt('\n');
	}
}

void USART1_transmit_int(int num) {
	char buffer[10]; //string for putting number in
	itoa(num, buffer, 10); // Convert integer to string
	USART1_transmitt_char_array(0, buffer, strlen(buffer));
}


/*   Handler for reading from buffer to make a message   */
void USART1buffer2message(){
	while (USART1RX_buffer_write != USART1RX_buffer_read && USART1message_available==0){	//If unread in buffer
		char c = USART1RX_buffer[USART1RX_buffer_read];	//Assign temp char
		USART1RX_buffer_read++;	//Increment read position
		
		if (USART1RX_buffer_read==51)	{	//Reset read position at end of array
			USART1RX_buffer_read=0;
		}
		
		if (c == '<'){	//Recognize start of message
			USART1marker_pos=0;	//Reset message position
			strcpy(USART1message,"");	//Delete previous message string
		}
		else if (c == '>'){	//Recognize end of message
			USART1message_available = 1;	//Set flag for available message
		}
		else{
			USART1message[USART1marker_pos]=c;	//Append to message string
			USART1marker_pos++;	//Increment message position
			
		}
	}
}

ISR(USART1_RXC_vect) {
	char tmp=USART1.RXDATAL;
	USART1RX_buffer[USART1RX_buffer_write]=tmp;
	USART1RX_buffer_write++;
	if (USART1RX_buffer_write==51)	{
		USART1RX_buffer_write=0;
	}
}


ISR(USART1_TXC_vect){	//Usart transmitt interrupt service routine
	if(USART1TX_buffer_read!=USART1TX_buffer_write){	//if there is not anything unread in buffer
		USART1.TXDATAL = USART1TX_buffer[USART1TX_buffer_read];	//Transmitt the next char
		USART1TX_buffer_read++;//Increment read position
		if (USART1TX_buffer_read ==(USART1TX_buffer_size+1)){//Reset position at end of array
			USART1TX_buffer_read=0;
		}
		USART1.STATUS |= USART_TXCIF_bm;	//reset interrupt flag
	}
	else// no problems
	USART1.STATUS |= USART_TXCIF_bm;	//reset interrupt flag
}

