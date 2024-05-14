/*
 * functions.h
 *
 * Created: 18.03.2024 22:55:39
 *  Author: zalto
 */ 


#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

void CLKCTRL_INTERNAL_24M_init();
void CLKCTRL_INTERNAL_16M_init();
void CLKCTRL_EXTERNAL_init();
void button_init(void);
void button_handler(void);
void message_handler_HMI(void);
void USART0message_handler(void);
void USART3message_handler(void);
void readbackemf(void);
void set_slow_decay(void);
void unset_slow_decay(void);
void start_motor(void);
void stop_motor(void);
uint16_t convert_period2rpm(uint32_t value);
uint16_t convert_int2percentage(uint16_t input, uint16_t input_max_value);
void sendupdate2HMI(void);
void sendupdate2USART0(void);
void sendupdate2USART3(void);
void analog_out_update(void);
void analog_to_dutycycle(uint16_t value);
uint16_t adc2voltage(uint16_t value);
void sendrpm2USART3(void);
void sendpid2USART3(float y0,float u,float p,float i,float d, float e);
void current_calibration(void);
void readmotorcurrent(void);
void readgeneratorcurrent(void);
void calculate_rpm(void);
#endif /* FUNCTIONS_H_ */