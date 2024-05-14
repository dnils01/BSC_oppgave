/*
 * config.h
 *
 * Created: 18.03.2024 18:49:24
 *  Author: zalto
 */ 


#ifndef CONFIG_H_
#define CONFIG_H_
//#define debugmessages //For a huge amount of debug messages
#define FRQ 16000000
#define F_CPU 16000000ul
#define USART3BAUDRATE 460800
#define AVG_FILTER_SIZE 2		//minimum 2
#define TCA0_period 400

#define ADC_GEN_V_ch 0
#define ADC_MOTOR_V_ch 1
#define ADC_MOTOR_I_ch 2
#define ADC_GEN_I_ch 3
#define ADC_AIN_ch 4
#define ADC_REF_VOLTAGE 2.5
#define CURRENT_PER_VOLTAGE 2000

#define MOTOR_MAX_RPM 4000

#endif /* CONFIG_H_ */