/*
 * usart.h
 */ 


#ifndef USART_H_
#define USART_H_

#include <avr/io.h>
#define USART3_TX_PIN_bp 0
#define USART3_RX_PIN_bp 1
#include <stdio.h>

char USART1message[50];
char USART1message_received[14];
volatile uint8_t USART1marker_pos;
volatile uint8_t USART1message_ready;
volatile uint8_t USART1message_available;


void USART1_init();
void USART1_transmitt(char c);
void USART1_transmitt_interrupt(char c);
void USART1_receive(void);
void USART1_transmitt_char_array(uint8_t addEndLine,char string[], uint8_t length);
void USART1_transmit_int(int num);
void USART1_transmitt_string(char string[]);
void USART1buffer2message(void);
#endif /* USART_H_ */