/*
 * adc.h
 *
 * Created: 11.09.2022 18:00:07
 *  Author: zalto
 */ 


#ifndef ADC_H_
#define ADC_H_
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>


void ADC0_init();
uint16_t ADC0_read_value(uint8_t channel);
uint16_t ADC0_read_assigned_value(uint8_t channel);
void ADC0_assign_value(void);
void ADC0_start(void);
void ADC0_stop(void);
void ADC0_change_input(uint8_t input);
#endif