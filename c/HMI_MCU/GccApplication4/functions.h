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
void LEDS_init(void);
void button_init(void);
void button_handler(void);
void led_handler(void);
void message_handler_HMI(void);
void update_variable(volatile uint16_t *var, uint16_t new_value);
void update_variable_uint8(volatile uint8_t *var, uint8_t new_value);
void SendStatus2otherMCU(void);
void GeneratorSetDutycycle(uint16_t duty);
uint16_t int2percentage(uint16_t input, uint16_t input_max_value);
#endif /* FUNCTIONS_H_ */