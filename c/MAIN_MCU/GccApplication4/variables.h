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
extern volatile uint8_t buttondown;
extern volatile uint8_t buttonup;
extern volatile uint8_t buttonenter;

extern volatile uint8_t USARTcontrol;

//TCB flags
extern volatile uint8_t tcb0_new_reading;
extern volatile uint8_t tcb1_new_reading;

extern volatile uint8_t regulatormode;
extern volatile uint8_t regulatormode_override;
extern volatile uint8_t controlmodeUSART;
extern volatile uint8_t controlmodeAIN;


//PID parameters
extern volatile uint16_t adjustsetpoint;
extern volatile uint16_t adjustkp;
extern volatile uint16_t adjustki;
extern volatile uint16_t adjustkd;
extern volatile uint16_t adjustgenerator;

extern volatile uint16_t rpm_uint;
extern volatile uint16_t motor_current_value;
extern volatile uint16_t back_emf_value;
extern volatile uint16_t adjustgenerator_override;

extern volatile uint8_t slowdecay_selected;
extern volatile uint16_t motor_duty_cycle;
extern volatile uint16_t motor_duty_cycle_old;
extern volatile uint8_t slowdecay_selected;
//Motor and generator readings variables
extern volatile uint16_t generator_voltage;
extern volatile uint16_t generator_current;

extern volatile uint16_t motor_current;
extern volatile uint16_t motor_voltage;
extern volatile uint16_t motor_backemf;
extern volatile uint16_t motor_rpm;
extern volatile uint16_t motor_rpm0;
extern volatile uint16_t motor_rpm1;
extern volatile uint8_t motor_rpm_updated;

extern volatile uint32_t tcb0_last_time;
extern volatile uint32_t tcb1_last_time;
extern volatile uint8_t time2updateRPM;
extern volatile uint8_t time2updatevi;
extern volatile uint8_t AIN_selected;

extern volatile uint8_t outputUSART;
extern volatile uint8_t outputAOUT;
extern volatile uint8_t regulator_parameters_updated;



#endif /* VARIABLES_H_ */