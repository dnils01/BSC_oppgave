/*
 * usart.h
 */ 


#ifndef USART_H_
#define USART_H_

#include <avr/io.h>
#define USART3_TX_PIN_bp 0
#define USART3_RX_PIN_bp 1
#include <stdio.h>

char message[50];
char message_received[14];
volatile uint8_t marker_pos;
volatile uint8_t message_ready;
volatile uint8_t message_available;

char USART0message[50];
char USART0message_received[14];
volatile uint8_t USART0marker_pos;
volatile uint8_t USART0message_ready;
volatile uint8_t USART0message_available;
void USART0_init();
void USART3_init();
void USART0_transmitt(char c);
void USART3_transmitt(char c);
void USART0_transmitt_interrupt(char c);
void USART3_transmitt_interrupt(char c);
void USART0_receive(void);
void USART3_receive(void);
/* printing an array of chars*/
void USART0_transmitt_char_array(uint8_t addEndLine,char string[], uint8_t length);
void USART0_transmit_int(int num);
void USART3_transmitt_char_array(uint8_t addEndLine,char string[], uint8_t length);
void USART3_transmit_int(int num);
void USART0_transmitt_string(char string[]);
void USART3_transmitt_string(char string[]);

/* preparing for printf */
//static uint8_t USART3_transmitt_printf(char c, FILE *stream);
//static FILE new_std_out = FDEV_SETUP_STREAM(USART3_transmitt_printf, NULL, _FDEV_SETUP_WRITE);

void buffer2message(void);
void USART0buffer2message(void);

void sw_read_usart(void);
void check_message(void);
#endif /* USART_H_ */