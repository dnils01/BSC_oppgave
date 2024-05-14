/*
 * functions.c
 *
 * Created: 18.03.2024 22:55:51
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
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

extern const address = 0x27;

volatile uint8_t motor_running=0;


volatile uint8_t regulator_parameters_updated=0;
volatile uint16_t adjustsetpoint;
volatile uint16_t adjustkp;
volatile uint16_t adjustki;
volatile uint16_t adjustkd;
volatile uint16_t adjustgenerator;


volatile uint16_t rpm_uint;
volatile uint16_t rpm_uint_old=9999;

volatile uint16_t motor_current=0;
volatile uint16_t motor_current_old=0;
volatile uint16_t motor_current_old3=0;

volatile uint16_t back_emf_value=0;
volatile uint16_t back_emf_value_old=0;
volatile uint16_t back_emf_value_old3=0;

volatile uint16_t motor_voltage=0;
volatile uint16_t motor_voltage_old=0;
volatile uint16_t motor_voltage_old3=0;

volatile uint16_t motor_backemf=0;
volatile uint16_t motor_backemf_old=0;
volatile uint16_t motor_backemf_old3=0;

volatile uint16_t generator_voltage=0;
volatile uint16_t generator_voltage_old=0;
volatile uint16_t generator_current=0;
volatile uint16_t generator_current_old;

volatile uint16_t adjustgenerator_override=0;
volatile uint16_t adjustgenerator_override_old=0;

volatile uint16_t generator_voltage_adc_reading;
volatile uint16_t motor_voltage_adc_reading;
volatile uint16_t motor_current_adc_reading;
volatile uint16_t generator_current_adc_reading;
volatile uint16_t analog_in_adc_reading;

volatile uint8_t slowdecay_selected=1;
volatile uint8_t slowdecay_selected_old=1;
volatile uint16_t motor_duty_cycle=0;
volatile uint16_t motor_duty_cycle_old=0;
volatile uint16_t motor_rpm;
volatile uint16_t motor_rpm_old=9999;
volatile uint16_t motor_rpm0=0;
volatile uint16_t motor_rpm1=0;
volatile uint8_t time2updateRPM=1;
volatile uint8_t time2readADC=1;
volatile uint8_t counter4adc=1;
volatile uint32_t last_update_usart0=0;

volatile uint32_t last_time_rpm_updated=0;
volatile uint8_t motor_rpm_updated=0;
volatile uint8_t controlmodeUSART=0;
volatile uint8_t controlmodeAIN=0;
volatile uint8_t io_changed=0;
volatile uint8_t regulatormode=0;
volatile uint8_t regulatormode_override=0;
volatile uint8_t outputUSART=1;
volatile uint8_t outputAOUT=0;

volatile uint16_t adc_motor_current_offset;

void CLKCTRL_INTERNAL_24M_init(){
	ccp_write_io((void *)&(CLKCTRL.OSCHFCTRLA) ,CLKCTRL_FRQSEL_24M_gc         /* 24 MHz internal clock */
	| 0 << CLKCTRL_AUTOTUNE_bp /* Auto-Tune enable: disabled */
	| 0 << CLKCTRL_RUNSTDBY_bp /* Run standby: disabled */);
}

void CLKCTRL_INTERNAL_16M_init(){
	ccp_write_io((void *)&(CLKCTRL.OSCHFCTRLA) ,CLKCTRL_FRQSEL_16M_gc         /* 16 MHz internal clock */
	| 0 << CLKCTRL_AUTOTUNE_bp /* Auto-Tune enable: disabled */
	| 0 << CLKCTRL_RUNSTDBY_bp /* Run standby: disabled */);
}

void CLKCTRL_EXTERNAL_init(){	
ccp_write_io (( void *) & (CLKCTRL.XOSCHFCTRLA), CLKCTRL_FRQRANGE_16M_gc | CLKCTRL_ENABLE_bm );
ccp_write_io (( void *) & (CLKCTRL.MCLKCTRLA), CLKCTRL_CLKSEL_EXTCLK_gc);
ccp_write_io (( void *) & (CLKCTRL.MCLKCTRLB), CLKCTRL_PDIV_4X_gc | CLKCTRL_PEN_bm);
}

void USART3message_handler(void){
	buffer2message();	//Read from buffer and make a message
	if (message_available==1){
		if(message[0]=='D'){	//motor duty cycle command
			char tmp[4];
			tmp[0]= message[1];
			tmp[1]= message[2];
			tmp[2]= message[3];
			motor_duty_cycle=atoi(tmp);
			if (motor_duty_cycle>=100){	//limit value
				motor_duty_cycle=100;
			}
			controlmodeUSART=1;
			controlmodeAIN=0;
			regulatormode=0;
			regulatormode_override=0;
		}
		else if(message[0]=='G'){	//Generator duty cycle command
			char tmp[4];
			tmp[0]= message[1];
			tmp[1]= message[2];
			tmp[2]= message[3];
			adjustgenerator_override=atoi(tmp);
			if (adjustgenerator_override>=100){	//limit value
				adjustgenerator_override=100;
			}
			char tmp2[4];
			sprintf(tmp2, "%03u", adjustgenerator_override);
			USART0_transmitt_string("<G");
			USART0_transmitt_string(tmp2);
			USART0_transmitt('>');
		}
		else if (message[0]=='A'){	//ADC control command			
			if (message[1]=='0'){	//ADC control disable
				controlmodeAIN=0;
			}
			else if (message[1]=='1'){	//ADC control enable
				controlmodeAIN=1;
				controlmodeUSART=0;	
				regulatormode=0;		
				regulatormode_override=0;			
			}			
		}
		else if (message[0]=='a'){	//ADC output command
			if (message[1]=='0'){	//ADC output disable
				outputAOUT=0;
			}
			else if (message[1]=='1'){	//ADC output enable
				outputAOUT=1;				
			}
		}
	
		else if (message[0]=='U'){	//ADC control command			
			if (message[1]=='0'){	//ADC control disable
				outputUSART=0;
			}
			else if (message[1]=='1'){	//ADC control enable
				controlmodeAIN=1;
				controlmodeUSART=0;
				regulatormode=0;
				regulatormode_override=0;
			}
		}
		else if (message[0]=='S'){	//slow decay command
			if (message[1]=='0'){
				slowdecay_selected=0;
				unset_slow_decay();
			}
			else if (message[1]=='1'){
				slowdecay_selected=1;
				set_slow_decay();
			}
		}		
		else if(message[0]=='R'){	//regulatormode commnad
			if(message[1]=='0'){
				regulatormode_override=0;
				regulatormode=0;
			}
			else if(message[1]=='1'){
				regulatormode_override=1;
				controlmodeAIN=0;
				controlmodeUSART=0;
				regulatormode_override=1;
				
			}
		}
		else if(message[0]=='y'){	//setpoint parameter
			char tmp[6];
			tmp[0]=	message[1];	
			tmp[1]=	message[2];	
			tmp[2]=	message[3];	
			tmp[3]=	message[4];	
			adjustsetpoint= atoi(tmp);
			regulator_parameters_updated=1;
		}
		else if(message[0]=='p'){	//kp parameter
			char tmp[6];
			tmp[0]=	message[1];
			tmp[1]=	message[2];
			tmp[2]=	message[3];
			tmp[3]=	message[4];
			adjustkp= atoi(tmp);
			regulator_parameters_updated=1;
		}
		else if(message[0]=='i'){	//ki parameter
			char tmp[6];
			tmp[0]=	message[1];
			tmp[1]=	message[2];
			tmp[2]=	message[3];
			tmp[3]=	message[4];
			adjustki= atoi(tmp);
			regulator_parameters_updated=1;
		}
		else if(message[0]=='d'){	//kd parameter
			char tmp[6];
			tmp[0]=	message[1];
			tmp[1]=	message[2];
			tmp[2]=	message[3];
			tmp[3]=	message[4];
			adjustkd= atoi(tmp);
			regulator_parameters_updated=1;
		}
		strcpy(message, "");
		message_available=0;
	}	
}

void USART0message_handler(void){	
	USART0buffer2message();
	if(USART0message_available){
			if (USART0message[0]=='A'){	//io message
				if(USART0message[1]=='0'){
					if(controlmodeUSART==1){
						controlmodeUSART=1;
						controlmodeAIN=0;
					}
				}
				else if(USART0message[1]=='1'){
					if(controlmodeUSART==0){						
						controlmodeAIN=0;
						controlmodeUSART=1;	
						regulatormode=0;
						regulatormode_override=0;					
					}
				}
				
				if(USART0message[2]=='0'){
					if(outputUSART==1){
						outputUSART=0;
					}
				}
				else if(USART0message[2]=='1'){
					if(outputUSART==0){
						outputUSART=1;
					}
				}
				if(USART0message[3]=='0'){
					if(controlmodeAIN==1){
						controlmodeAIN=0;
					}
				}
				else if(USART0message[3]=='1'){
					if(controlmodeAIN==0){
						controlmodeAIN=1;
						controlmodeUSART=0;
						regulatormode=0;
						regulatormode_override=0;
					}
				}
				if(USART0message[4]=='0'){
					if(outputAOUT==1){
						outputAOUT=0;
					}
				}
				else if(USART0message[4]=='1'){
					if(outputAOUT==0){
						outputAOUT=1;
					}
				}	
			}
			else if(USART0message[0]=='R'){	//regulator mode status
				if(USART0message[1]=='0'){	
					if (regulatormode==1){
						regulatormode=0;
					}
				}
				else if(USART0message[1]=='1'){
					if (regulatormode==0){
						regulatormode=1;
						controlmodeUSART=0;
						controlmodeAIN=0;
					}
				}
				char tmp[8];
				regulator_parameters_updated=1;
				uint16_t tmpvar;
				tmp[0]=USART0message[2];	//start of adjustsetpoint
				tmp[1]=USART0message[3];
				tmp[2]=USART0message[4];
				tmp[3]=USART0message[5];
				tmp[4]='\0';
				tmpvar= atoi(tmp);
				if(tmpvar!=adjustsetpoint){
					adjustsetpoint=tmpvar;
				}
				tmpvar=0;
				strcpy(tmp, "");
				tmp[0]=USART0message[6];	//start of adjustkp
				tmp[1]=USART0message[7];
				tmp[2]=USART0message[8];
				tmp[3]=USART0message[9];
				tmp[4]='\0';
				tmpvar= atoi(tmp);
				if(tmpvar!=adjustkp){
					adjustkp = tmpvar;
				}
				
				strcpy(tmp, "");
				tmp[0]=USART0message[10];	//start of adjustki
				tmp[1]=USART0message[11];
				tmp[2]=USART0message[12];
				tmp[3]=USART0message[13];
				tmp[4]='\0';
				tmpvar= atoi(tmp);
				if(tmpvar!=adjustki){
					adjustki = tmpvar;
				}
				strcpy(tmp, "");
				tmp[0]=USART0message[14];	//start of adjustkd
				tmp[1]=USART0message[15];
				tmp[2]=USART0message[16];
				tmp[3]=USART0message[17];
				tmp[4]='\0';
				tmpvar= atoi(tmp);
				if(tmpvar!=adjustkd){
					adjustkd=tmpvar;
				}
				
			}					
				
			
			else if (USART0message[0]=='G'){
				char tmp[8];
				uint16_t tmpvar;
				tmp[0]=USART0message[1];	//start of adjustgenerator
				tmp[1]=USART0message[2];
				tmp[2]=USART0message[3];
				tmp[3]='\0';
				tmpvar= atoi(tmp);
				if(tmpvar!=adjustgenerator){
					adjustgenerator=tmpvar;
				}
			}			
			
		strcpy(USART0message, "");
		USART0message_available=0;
	}
}

void sendupdate2HMI(void){
	uint32_t now=read_tca_time();
	
	if ((now-last_update_usart0)>=250000){	//check if enough time has passed since last update
		last_update_usart0=now;
		
		if(motor_rpm_old != motor_rpm){		//Check if variable has changed
			motor_rpm_old=motor_rpm;
			char tmp1[8];
			sprintf(tmp1, "%04u", motor_rpm);
			USART0_transmitt_string("<R");
			USART0_transmitt_string(tmp1);
			USART0_transmitt_string(">");
		}
		if (motor_current != motor_current_old){	//Check if variable has changed
			motor_current_old=motor_current;
			char tmp2[8];
			sprintf(tmp2, "%04u", motor_current);
			USART0_transmitt_string("<I");
			USART0_transmitt_string(tmp2);
			USART0_transmitt_string(">");
		}
		if(motor_backemf != motor_backemf_old){	//Check if variable has changed
			motor_backemf_old=motor_backemf;
			char tmp3[8];
			sprintf(tmp3, "%04u", motor_backemf);
			USART0_transmitt_string("<B");
			USART0_transmitt_string(tmp3);
			USART0_transmitt_string(">");
		}
		if(adjustgenerator_override != adjustgenerator_override_old){	//Check if variable has changed
			adjustgenerator_override_old=adjustgenerator_override;
			char tmp4[8];
			sprintf(tmp4, "%03u", adjustgenerator_override);
			USART0_transmitt_string("<G");
			USART0_transmitt_string(tmp4);
			USART0_transmitt_string(">");
		}
		if(io_changed==1){
			USART0_transmitt('<');	//update io status
			USART0_transmitt('A');
			USART0_transmit_int(controlmodeUSART);
			USART0_transmit_int(outputUSART);
			USART0_transmit_int(controlmodeAIN);
			USART0_transmit_int(outputAOUT);
			USART0_transmitt('>');
		}
		char tmp5[6];
		sprintf(tmp5, "%03u", motor_duty_cycle);
		USART0_transmitt_string("<D");
		USART0_transmitt_string(tmp5);
		USART0_transmitt_string(">");
		
	}	
}

void sendrpm2USART3(void){
	if (time2updateRPM == 1){
		time2updateRPM = 0;
		char tmp[8];
		sprintf(tmp, "%04u", motor_rpm);
		USART3_transmitt_string("<R");
		USART3_transmitt_string(tmp);
		USART3_transmitt('>');
		USART3_transmitt('\n');
	}	
}

void sendui2USART3(void){	
	if (1){
		char tmp[8];
		back_emf_value_old3=back_emf_value;
		sprintf(tmp, "%04u", back_emf_value);
		USART3_transmitt_string("<b");
		USART3_transmit_int(motor_backemf);
		USART3_transmitt('>');
	}
	if (1)	{
		char tmp[8];
		motor_voltage_old3=motor_voltage;
		sprintf(tmp, "%04u", motor_voltage);
		USART3_transmitt_string("<V");
		USART3_transmit_int(motor_voltage);
		USART3_transmitt('>');
	}
	
	if (motor_current != motor_current_old3){
		motor_current_old3=motor_current;
		char tmp[8];
		sprintf(tmp, "%04u", motor_current);
		USART3_transmitt_string("<I");
		USART3_transmit_int(motor_current);
		USART3_transmitt('>');
	}
	
	if (generator_voltage != generator_voltage_old)	{
		char tmp[8];
		generator_voltage_old=generator_voltage;
		sprintf(tmp, "%04u", generator_voltage);
		USART3_transmitt_string("<v");
		USART3_transmit_int(generator_voltage);
		USART3_transmitt('>');
	}
	
	if (generator_current != generator_current_old){
		generator_current_old=generator_current;
		char tmp[8];
		sprintf(tmp, "%04u", generator_current);
		USART3_transmitt_string("<I");
		USART3_transmit_int(generator_current);
		USART3_transmitt('>');
	}
}

void sendpid2USART3(float y0, float u, float p, float i, float d, float e){ //send pid status
	char tmp[8];
	sprintf(tmp, "%04u", (int16_t)e);
	USART3_transmitt_string("<e");
	USART3_transmitt_string(tmp);
	USART3_transmitt('>');
	strcpy(tmp,"");
	
	sprintf(tmp, "%04u", (int16_t)u);
	USART3_transmitt_string("<u");
	USART3_transmitt_string(tmp);
	USART3_transmitt('>');	
	strcpy(tmp,"");
	sprintf(tmp, "%04u", (int16_t)y0);
	USART3_transmitt_string("<z");
	USART3_transmitt_string(tmp);
	USART3_transmitt('>');	
	strcpy(tmp,"");
	sprintf(tmp, "%04u", (uint16_t)p);
	USART3_transmitt_string("<p");
	USART3_transmitt_string(tmp);
	USART3_transmitt('>');
	strcpy(tmp,"");
	sprintf(tmp, "%04u", (uint16_t)i);
	USART3_transmitt_string("<i");
	USART3_transmitt_string(tmp);
	USART3_transmitt('>');
	strcpy(tmp,"");
	sprintf(tmp, "%04u", (uint16_t)d);
	USART3_transmitt_string("<d");
	USART3_transmitt_string(tmp);
	USART3_transmitt('>');
	strcpy(tmp,"");
	USART3_transmitt('\n');
}


uint16_t convert_int2percentage(uint16_t input, uint16_t input_max_value){
	float percentage = ((float)input * (100.0 / (float) input_max_value));
	uint16_t int_value=(uint16_t)percentage;
	return int_value;
}


void calculate_rpm(void){ // Calculate RPM from TCBperiod
	if (tcb0_new_reading==1&&tcb1_new_reading==1){
		motor_rpm0=convert_period2rpm(TCB0_read_period());	//calculate rpm for sensor 1
		motor_rpm1=convert_period2rpm(TCB1_read_period());	//calculate rpm for sensor 2
		motor_rpm = (motor_rpm0+motor_rpm1)/2;
		tcb0_new_reading=0;
		tcb1_new_reading=0;
		int16_t tmp_rpm_diff = (int16_t)motor_rpm0-(int16_t)motor_rpm1; 		
		if(motor_rpm0==0 && motor_rpm1!=0){
			if(motor_rpm1<=MOTOR_MAX_RPM){
				motor_rpm=motor_rpm1;
			}
			else{
				motor_rpm=0;
			}
		}
		else if(motor_rpm0!=0 && motor_rpm1==0){
			if(motor_rpm0<=MOTOR_MAX_RPM){
				motor_rpm=motor_rpm0;
			}
			else{
				motor_rpm=0;
			}
		}
		else if(motor_rpm>=MOTOR_MAX_RPM){
			motor_rpm= motor_rpm_old;
			if (motor_rpm_old>= MOTOR_MAX_RPM){
				motor_rpm=0;
			}
		}
		motor_rpm_updated=1;
		time2updateRPM=1;
		last_time_rpm_updated=read_tca_time();
		
		if ((motor_rpm >=100) && (motor_rpm <=1300))	//If in area for noise
		{
			if (tmp_rpm_diff >= 200){		// if difference between sensors are over limit for noise
				motor_rpm = motor_rpm1;
			}
			else if (tmp_rpm_diff <=-200){	// if difference between sensors are over limit for noise
				motor_rpm = motor_rpm0;
			}
		}
		else if (motor_rpm>=1300){
			if (tmp_rpm_diff>=200){
				motor_rpm= motor_rpm0;
			}
			else if(tmp_rpm_diff<=-200){
				motor_rpm=motor_rpm1;
			}
		}		
	}	
	else if (read_tca_time()-last_time_rpm_updated>=100000){
		motor_rpm=0;
		motor_rpm_updated=1;		
		time2updateRPM=1;
		last_time_rpm_updated=read_tca_time();
	}		
}

void start_motor(void){
	if (motor_running==0){
		if(slowdecay_selected==1){
			unset_slow_decay();
			set_slow_decay();		
		}
		else if (slowdecay_selected==0){
			unset_slow_decay();
		}
		
		motor_running=1;
	}
}

void change_motor_duty_cycle(uint16_t dutycycle){	//function for changing duty cycle based on uint
	if(dutycycle>=100){		//limiting max value of dutycycle
		dutycycle=100;
	}
	
	uint16_t tmp = dutycycle*4;	//convert dutycycle to cmp value
	if(slowdecay_selected==1){
		TCA0_PWM_set_duty(400-tmp);		//Make adjustment to duty cycle of motor.
	}
	else if(slowdecay_selected==0){
		TCA0_PWM_set_duty(tmp);
	}	
}

void stop_motor(void){
	if (motor_running==1){
		//TCA0_PWM_disable();
		TCA0.SINGLE.CTRLB &= ~TCA_SINGLE_CMP0EN_bm;
		TCA0.SINGLE.CTRLB &= ~TCA_SINGLE_CMP2EN_bm;
		unset_slow_decay();
		PORTA.OUTCLR |= PIN2_bm;		
		motor_running=0;
	}
}

void set_slow_decay(void){	
	if (slowdecay_selected_old==0){
		slowdecay_selected_old=0;
		PORTE.DIR |= PIN0_bm;
		PORTA.OUT |= PIN2_bm;
		PORTE.OUT |= PIN0_bm;
		//TCA0.SINGLE.CTRLB &= !TCA_SINGLE_CMP2EN_bm;
		PORTMUX.TCAROUTEA = PORTMUX_TCA0_PORTE_gc;
		_delay_us(50);
		TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP0EN_bm;
		TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;	//Enable TC0
		change_motor_duty_cycle(motor_duty_cycle);
	}	
}
void unset_slow_decay(void){
	if(slowdecay_selected_old==1){
		slowdecay_selected_old=0;
		PORTA.DIRSET = PIN0_bm|PIN2_bm;
		PORTE.OUT &= !PIN0_bm;
		PORTE.DIR &= !PIN0_bm;
		PORTMUX.TCAROUTEA = PORTMUX_TCA0_PORTA_gc;
		_delay_us(50);
		TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP2EN_bm;
		TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;	//Enable TC0
		change_motor_duty_cycle(motor_duty_cycle);
	}
}

uint16_t convert_period2rpm(uint32_t value){
	float floatvalue = 0;		
	if(value != 0){	//If input value is not 0, calculate rpm.
		floatvalue = (1000000/(float)value)*AVG_FILTER_SIZE;	
	}		
	return (uint16_t)floatvalue;
}

void current_calibration(){
	ADC0_change_input(ADC_MOTOR_I_ch);
	_delay_us(100);
	uint16_t tmp=ADC0_read_value();
	tmp+=ADC0_read_value();
	tmp+=ADC0_read_value();
	adc_motor_current_offset = tmp/3; 
}

void readmotorcurrent(void){
	ADC0_change_input(ADC_MOTOR_I_ch);
	_delay_us(10);
	uint16_t tmp=4095-ADC0_read_value();
	uint32_t tmp_upper = (uint32_t)tmp*ADC_REF_VOLTAGE*CURRENT_PER_VOLTAGE; 
	uint32_t tmp_lower = 4095;
	motor_current = (uint16_t) (tmp_upper/tmp_lower);
}
void readgeneratorcurrent(void){
	ADC0_change_input(ADC_GEN_I_ch);
	_delay_us(10);
	uint16_t tmp=4095-ADC0_read_value();
	uint32_t tmp_upper = (uint32_t)tmp*ADC_REF_VOLTAGE*CURRENT_PER_VOLTAGE;
	uint32_t tmp_lower = 4095;
	generator_current = (uint16_t) (tmp_upper/tmp_lower);
}

void analog_out_update(void){		//function for updating output based on rpm
	float input2=((float)motor_rpm*1023);
	float input3 =input2/(4000);
	float output = (uint16_t)input3;
	DAC0_setvalue(output);
}

void analog_to_dutycycle(uint16_t value){	//function to update duty cycle based on analog input
	uint32_t big_value = (uint32_t) value;
	big_value = (big_value * 1000*100)/(4095*1000);
	change_motor_duty_cycle((uint16_t)big_value);
	
}

void readbackemf(void){
	if (motor_duty_cycle !=0){
		ADC0_change_input(ADC_MOTOR_V_ch);
		if (slowdecay_selected==1){
			change_motor_duty_cycle(100);	//Get ready for voltage measurement
			_delay_us(50);
			motor_voltage = adc2voltage(ADC0_read_value());
			PORTA.OUTCLR |=PIN2_bm;
			_delay_us(100);
			motor_backemf = adc2voltage(ADC0_read_value());
			PORTA.OUTSET |=PIN2_bm;
		}
		else if(slowdecay_selected==0){
			change_motor_duty_cycle(100);
			_delay_us(50);
			motor_voltage = adc2voltage(ADC0_read_value());
			change_motor_duty_cycle(0);
			PORTA.DIRCLR |=PIN2_bm;
			PORTE.DIRCLR |=PIN0_bm;
			_delay_us(300);
			motor_backemf = adc2voltage(ADC0_read_value());
			PORTA.DIRSET |=PIN2_bm;
			PORTE.DIRSET |=PIN0_bm;
		}
		start_motor();
		change_motor_duty_cycle(motor_duty_cycle);
	}
}

uint16_t adc2voltage(uint16_t value){	//function to convert adc reading to voltage
	float big_value = ((float) value)/3.276;	//assuming max voltage of 12.5V -> 4095/12.5=327.6
	return (uint16_t)big_value;
}

void adcreading(void){
	if (time2readADC==1){
		if(AIN_selected==0){
			generator_voltage_adc_reading = ADC0_read_value();
			time2readADC=0;
		}
		else if(AIN_selected==1){
			motor_voltage_adc_reading = ADC0_read_value();
			time2readADC=0;
		}
		else if(AIN_selected==2){
			motor_current_adc_reading = ADC0_read_value();
			time2readADC=0;
		}
		else if (AIN_selected==3){
			generator_current_adc_reading = ADC0_read_value();
			time2readADC=0;
		}
	}
	else if (time2readADC==0){
		time2readADC=1;
		if(AIN_selected==0){
			ADC0_change_input(1);
		}
		else if(AIN_selected==1){
			ADC0_change_input(2);
		}
		else if(AIN_selected==2){
			ADC0_change_input(3);
		}
		else if (AIN_selected==3){
			ADC0_change_input(4);
		}
	}
}