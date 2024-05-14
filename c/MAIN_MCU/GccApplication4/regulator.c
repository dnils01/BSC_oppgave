/*
 * regulator.c
 *
 * Created: 20.03.2024 12:05:18
 *  Author: zalto
 */ 
#include "adc.h"
#include "config.h"
#include "dac.h"


#include "functions.h"
#include "regulator.h"
#include "tca.h"
#include "tcb.h"
#include "usart.h"
#include "variables.h"

#include <avr/cpufunc.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>


volatile float y0_last=0.0;

volatile float integral_limit =4000.0;


void pid_init(PIDregulator *pid, float y0, float y, float kp, float ki, float kd){		//initialize pid parameters
	pid->y0=y0;
	pid->y=y;
	pid->kp=kp;
	pid->ki=ki;
	pid->kd=kd;	
	pid->u=0;
}
void pid_change_parameters(PIDregulator *pid, float y0, float kp, float ki, float kd){		//change pid parameters
	pid->y0=y0;	
	pid->kp=kp;
	pid->ki=ki;
	pid->kd=kd;
	//pid->error_integral=0;
}

void pid_regulator(PIDregulator *pid){
	
	pid->error = pid->y0 - pid->y;
	//Check if error is unreasonable
	if (pid->error>=MOTOR_MAX_RPM){
		pid->error=pid->error_last;
	}
	else if (pid->error<=-MOTOR_MAX_RPM){
		pid->error=pid->error_last;
	}
	float p, i, d;
	
	float timestep = (pid->y/60)/60;
	if (timestep==0){	//if timestep would be wrong, assume some value
		timestep=1;
	}
	p = pid->kp*pid->error;		//P of PID
	
	pid->error_integral+=(pid->error*(timestep));
	//Anti-windup for regulator
	if (pid->error_integral>=integral_limit){	
		pid->error_integral=integral_limit;
	}
	else if (pid->error_integral<=-integral_limit){
		pid->error_integral=-integral_limit;
	}
	
	//Anti windup for when y crossing y0	//NB! Does not work as intended!
	/*if (pid->error >=0 && pid->error_last<=0){	
		pid->error_integral=0;
	}
	else if (pid->error <=0 && pid->error_last>=0){
		pid->error_integral=0;
	}*/
	i = pid->ki * pid->error_integral;	//I of PID
	
	d = pid->kd * ((pid->error-pid->error_last)/(timestep)); //D of PID
	pid->error_last =pid->error;
	//Sum of all parts of PID
	float output= p+i+d;	
	//Limiting output
	if (output>100){	
		output=100;
	}
	else if (output <0){
		output=0;
	}
	//Update variables
	pid->y_last=pid->y;	
	pid->u=output;
}





