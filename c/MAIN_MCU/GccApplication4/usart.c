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


uint8_t MAX_MESSAGE_SIZE =255;	
volatile char transmitt_buffer[50];
volatile char RX_buffer[50];
volatile char TX_buffer[50];
volatile uint8_t RX_buffer_write=0;
volatile uint8_t RX_buffer_read=0;
volatile uint8_t TX_buffer_write=0;
volatile uint8_t TX_buffer_read=0;
volatile uint8_t next = 0;
volatile uint8_t last = 0;
volatile uint8_t message_available = 0;
volatile uint8_t message_ready=0;
uint8_t TX_buffer_size=50;

volatile char USART0transmitt_buffer[50];
volatile char USART0RX_buffer[50];
volatile char USART0TX_buffer[50];
volatile uint8_t USART0RX_buffer_write=0;
volatile uint8_t USART0RX_buffer_read=0;
volatile uint8_t USART0TX_buffer_write=0;
volatile uint8_t USART0TX_buffer_read=0;
volatile uint8_t USART0next = 0;
volatile uint8_t USART0last = 0;
volatile uint8_t USART0message_available = 0;
volatile uint8_t USART0message_ready=0;
uint8_t USART0TX_buffer_size=50;




volatile uint8_t marker_pos = 0;


void USART0_init(){
	USART0.BAUD = 556/2;		
	USART0.CTRLB |= USART_RXEN_bm | USART_TXEN_bm | USART_RXMODE_CLK2X_gc;//USART_RXMODE_CLK2X_gc; //enable RX TX
	PORTA.DIR |= (1<<4);	// PA4 as output
	PORTA.PINCTRLSET |= (1<<5); //PA5 as input
	USART0.CTRLA |= USART_RXCIE_bm; // interrupt on RX
	
	
	USART0.CTRLC = USART_SBMODE_2BIT_gc;
	
	//USART0.CTRLA |= USART_TXCIE_bm; // interrupt on TX 
	USART0.CTRLC = USART_CHSIZE_8BIT_gc;
	PORTMUX.USARTROUTEA |= PORTMUX_USART0_ALT1_gc;
	#ifdef debugmessages
	USART3_transmitt_char_array(0, "USART0 initialized\n", strlen("USART0 initialized\n"));
	#endif
}

void USART3_init(){
	USART3.BAUD = 556/2;
	USART3.CTRLB |= USART_RXEN_bm | USART_TXEN_bm | USART_RXMODE_CLK2X_gc;//USART_RXMODE_CLK2X_gc; //enable RX TX
	PORTB.DIR |= (1<<USART3_TX_PIN_bp);
	//PORTB.PINCONFIG |= PORT_PULLUPEN_bm;
	PORTB.PINCTRLSET |= (1<<USART3_RX_PIN_bp);
	/* interrupt on RX*/
	USART3.CTRLA |= USART_RXCIE_bm;
	
	
	USART3.CTRLC = USART_SBMODE_2BIT_gc;
	/* interrupt on TX */
	//USART3.CTRLA |= USART_TXCIE_bm;
	USART3.CTRLC = USART_CHSIZE_8BIT_gc;
}

void USART0_transmitt(char c){
	USART0TX_buffer[USART0TX_buffer_write] = c;	//Write char to buffer.
	USART0TX_buffer_write++; //Increment write position
	if (USART0TX_buffer_write == (USART0TX_buffer_size+1)){	//Reset position at end of array
		USART0TX_buffer_write=0;
	}
	
	if((USART0.STATUS & USART_DREIF_bm)){	//If empty data register, start the transmitting
		USART0.TXDATAL=USART0TX_buffer[USART0TX_buffer_read];	//Transmitt the next char
		USART0TX_buffer_read++;	//Increment read position
		if (USART0TX_buffer_read ==(USART0TX_buffer_size+1)){	//Reset position at end of array
			USART0TX_buffer_read=0;
		}
	}
	_delay_us(18);  //Delay because of problems with transmitting strings, if not there only 3 first chars transmitted.
}


void USART3_transmitt(char c){
	TX_buffer[TX_buffer_write] = c;	//Write char to buffer.
	TX_buffer_write++; //Increment write position
	if (TX_buffer_write == (TX_buffer_size+1)){	//Reset position at end of array
		TX_buffer_write=0;
	}
	
	if((USART3.STATUS & USART_DREIF_bm)){	//If empty data register, start the transmitting
		USART3.TXDATAL=TX_buffer[TX_buffer_read];	//Transmitt the next char
		TX_buffer_read++;	//Increment read position
		if (TX_buffer_read ==(TX_buffer_size+1)){	//Reset position at end of array
			TX_buffer_read=0;
		}	
	}	
	_delay_us(18);  //Delay because of problems with transmitting strings, if not there only 3 first chars transmitted.
}


/* transmitting*/
void USART0_transmitt_string(char string[]){
	for(uint8_t i = 0; i<strlen(string); i++){
		USART0_transmitt(string[i]);
	}
}
void USART3_transmitt_string(char string[]){
	for(uint8_t i = 0; i<strlen(string); i++){
		USART3_transmitt(string[i]);
	}
}

void USART0_transmitt_char_array(uint8_t addEndLine,char string[], uint8_t length){
	for(uint8_t i = 0; i<length; i++){
		USART0_transmitt(string[i]);
	}
	if(addEndLine){
		USART0_transmitt('\n');
	}
}

void USART3_transmitt_char_array(uint8_t addEndLine,char string[], uint8_t length){
	for(uint8_t i = 0; i<length; i++){
		USART3_transmitt(string[i]);		
	}
	if(addEndLine){
		USART3_transmitt('\n');
	}	
}

void USART0_transmit_int(int num) {
	char buffer[10]; //string for putting number in
	itoa(num, buffer, 10); // Convert integer to string
	USART0_transmitt_char_array(0, buffer, strlen(buffer));
}

void USART3_transmit_int(int num) {
	char buffer[10]; //string for putting number in
	itoa(num, buffer, 10); // Convert integer to string
	USART3_transmitt_char_array(0, buffer, strlen(buffer));
}

/*   Handler for reading from buffer to make a message   */
void USART0buffer2message(void){
		
	if (USART0RX_buffer_write != USART0RX_buffer_read){	//If unread in buffer
		char c = USART0RX_buffer[USART0RX_buffer_read];	//Assign temp char
		USART0RX_buffer_read++;	//Increment read position
				
		if (USART0RX_buffer_read==51)	{	//Reset read position at end of array
			USART0RX_buffer_read=0;
		}
		
		if (c == '<'){	//Recognize start of message
			USART0marker_pos=0;	//Reset message position
			strcpy(USART0message,"");	//Delete previous message string
		}
		else if (c == '>'){	//Recognize end of message
			USART0message_available = 1;	//Set flag for available message
		}
		else{
			USART0message[USART0marker_pos]=c;	//Append to message string
			USART0marker_pos++;	//Increment message position
			
		}
	}
}

void buffer2message(void){
	if (RX_buffer_write != RX_buffer_read){	//If unread in buffer
		char c = RX_buffer[RX_buffer_read];	//Assign temp char
		RX_buffer_read++;	//Increment read position
			
		if (RX_buffer_read==51)	{	//Reset read position at end of array
			RX_buffer_read=0;
		}
		
		if (c == '<'){	//Recognize start of message
			marker_pos=0;	//Reset message position
			strcpy(message,"");	//Delete previous message string
		} 
		else if (c == '>'){	//Recognize end of message
			message_available = 1;	//Set flag for available message
		}
		else{
			message[marker_pos]=c;	//Append to message string
			marker_pos++;	//Increment message position
		
		}
	}
}
ISR(USART0_RXC_vect) {
	char tmp=USART0.RXDATAL;
	USART0RX_buffer[USART0RX_buffer_write]=tmp;	//write to buffer
	USART0RX_buffer_write++;
	if (USART0RX_buffer_write==51){		//if end of buffer
		USART0RX_buffer_write=0;
	}	
}

ISR(USART3_RXC_vect) {		
	char tmp=USART3.RXDATAL;
	RX_buffer[RX_buffer_write]=tmp;	//write to buffer
	RX_buffer_write++;
	if (RX_buffer_write==51){		//if end of buffer
		RX_buffer_write=0;
	}		
}

ISR(USART0_TXC_vect){	//Usart transmitt interrupt service routine
	if(USART0TX_buffer_read!=USART0TX_buffer_write){	//if there is not anything unread in buffer
		USART3.TXDATAL = USART0TX_buffer[USART0TX_buffer_read];	//Transmit the next char
		USART0TX_buffer_read++;//Increment read position
		if (USART0TX_buffer_read ==(USART0TX_buffer_size+1)){//Reset position at end of array
			USART0TX_buffer_read=0;
		}
		USART0.STATUS |= USART_TXCIF_bm;	//reset interrupt flag
	}
	else// no problems
	USART0.STATUS |= USART_TXCIF_bm;	//reset interrupt flag
}

ISR(USART3_TXC_vect){	//Usart transmitt interrupt service routine
	if(TX_buffer_read!=TX_buffer_write){	//if there is not anything unread in buffer
		USART3.TXDATAL = TX_buffer[TX_buffer_read];	//Transmitt the next char
		TX_buffer_read++;//Increment read position
		if (TX_buffer_read ==(TX_buffer_size+1)){//Reset position at end of array
			TX_buffer_read=0;
		}
		USART3.STATUS |= USART_TXCIF_bm;	//reset interrupt flag
	}
	else// no problems
	USART3.STATUS |= USART_TXCIF_bm;	//reset interrupt flag	
}



