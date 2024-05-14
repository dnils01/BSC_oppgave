/*
 * dac.h
 *
 * Created: 22.01.2024 13:08:00
 *  Author: zalto
 */ 


#ifndef DAC_H_
#define DAC_H_
#include <stdint.h>
void DAC0_init(void);

void DAC0_setvalue(uint16_t value);
void DAC0_enable(void);
void DAC0_disable(void);
#endif