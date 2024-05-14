/*
 * variables.h
 *
 * Created: 18.03.2024 23:04:39
 *  Author: zalto
 */ 


#ifndef VARIABLES_H_
#define VARIABLES_H_
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern const address;
extern uint8_t buttondown;
extern uint8_t buttonup;
extern uint8_t buttonenter;

extern uint8_t analog_in_status;
extern uint8_t analog_out_status;
extern uint8_t digital_in_status;
extern uint8_t digital_out_status;

extern uint8_t tcb0_new_reading;
extern uint8_t tcb1_new_reading;
extern uint8_t regulatormode;


extern volatile uint16_t dutycycle_value;
extern volatile uint16_t rpm_value;
extern volatile uint8_t rpm_value_write;
extern volatile uint16_t backemf_value;
extern volatile uint16_t current_value;

extern volatile uint8_t regulatormodestatus;
extern volatile uint16_t adjustsetpoint;
extern volatile uint16_t adjustkp;
extern volatile uint16_t adjustki;
extern volatile uint16_t adjustkd;
extern volatile uint16_t adjustgenerator;
extern volatile uint16_t adjustgenerator_old;
extern volatile uint8_t generatorenable;
extern volatile uint8_t generatorADCenable;

extern volatile uint8_t menuselected;
extern volatile uint8_t new_readings;

extern volatile uint8_t ADC_reading_ready;
extern volatile uint8_t time2sendData;

extern volatile uint8_t io_changed;
extern volatile uint8_t regulatormode_changed;
extern volatile uint8_t generator_changed;
#endif /* VARIABLES_H_ */