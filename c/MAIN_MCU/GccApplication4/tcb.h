/*
 * tcb.h
 *
 * Created: 15.01.2024 13:09:17
 *  Author: zalto
 */ 
#ifndef TCB_H_
#define TCB_H_
#include <stdint.h>

void TCB0_init(void);
void TCB1_init(void);
uint32_t TCB0_read_period(void);
uint32_t TCB1_read_period(void);


#endif /* TCA_H_ */