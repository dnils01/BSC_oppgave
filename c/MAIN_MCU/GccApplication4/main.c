#define F_CPU 16000000ul

#include <avr/cpufunc.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h> 

#include "adc.h"
#include "config.h"
#include "dac.h"
#include "init.h"
#include "functions.h"	
#include "regulator.h"
#include "tca.h"
#include "tcb.h"
#include "usart.h"
#include "variables.h"

volatile uint32_t now=0;
volatile uint16_t count = 0;
volatile uint8_t regulatormode_last=0;
volatile uint16_t countervariable=0;

int main(void)
{	Initialize_system();
	
	USART3_transmitt_string("Ready\n");
	PIDregulator pid;	//Make a struct of type pidregulator
	pid_init(&pid, 2000.0, 0.0, 0.540, 0.015, 0.0); // initialize regulator parameters y0,y,kp,ki,kd for pid
	
	sei();
	
	start_motor();
	_delay_ms(500);
	while(1){		
		//USART0message_handler(); //uncomment for when hmi mcu can have something to say
		USART3message_handler();
		calculate_rpm();		
		if(regulatormode==0 &&regulatormode_override==0){		
			if (regulatormode_last==1){		// If regulatormode have changed
				regulatormode_last=0;	
				change_motor_duty_cycle(0);
			}			
		}		
		else if((regulatormode==1) | (regulatormode_override==1)){			
			if (regulatormode_last==0){
				pid.error_integral=0.1;
			}
			regulatormode_last=1;			
			if (motor_rpm_updated == 1)	{	//if new reading available			
				pid.y = (float)motor_rpm;				
				pid_regulator(&pid);						
				change_motor_duty_cycle((uint16_t)pid.u);//Make adjustment to duty cycle of motor.	
				motor_rpm_updated=0;
				sendrpm2USART3();
				if (outputUSART==1)	{
					sendpid2USART3(pid.y0, pid.u, pid.p, pid.i, pid.d, pid.error);
				}				
				if(regulator_parameters_updated==1){
					regulator_parameters_updated=0;
					pid.y0=(float)adjustsetpoint;
					pid.kp=((float)adjustkp)/1000;
					pid.ki=((float)adjustki)/1000;
					pid.kd=((float)adjustkd)/1000;
				}
			}
		}			
		
		if (controlmodeUSART==1){			
			if (motor_duty_cycle != motor_duty_cycle_old){
				motor_duty_cycle_old=motor_duty_cycle;
				change_motor_duty_cycle(motor_duty_cycle);
			}						
		}
		
		if (controlmodeAIN==1){	//if Analog control is enabled			
			ADC0_change_input(ADC_AIN_ch);
			analog_to_dutycycle(ADC0_read_value());	
		}		
		
		if(outputUSART==1){			
			if(motor_rpm_updated==1){
				motor_rpm_updated=0;
				sendrpm2USART3();
				if(countervariable==30){
					countervariable=0;
					readbackemf();
					sendui2USART3();
					USART3_transmitt('\n');
				}
				countervariable++;				
			}			
		}
		if (outputAOUT==1){
			analog_out_update();
		}
		sendupdate2HMI();	
		/*uint32_t also_now = read_tca_time();
		uint16_t totaltime = (uint16_t)(also_now-now);
		USART3_transmitt_string("\nTotal loop time =  ");
		USART3_transmit_int(totaltime);
		USART3_transmitt('\n');
		now = read_tca_time();*/
	}	//End of while loop
}



