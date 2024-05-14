/*
 * tca.h
 *
 * Created: 11.03.2024 17:31:33
 *  Author: zalto
 */ 


#ifndef TCA_H_
#define TCA_H_
void tca0_init(void);
void TCA0_PWM_init(void);
void TCA0_PWM_enable(void);
void TCA0_PWM_disable(void);
void TCA0_PWM_set_duty(uint16_t);
void TCA0_PWM_set_period(uint16_t);

#endif /* TCA_H_ */
