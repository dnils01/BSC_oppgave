/*
 * regulator.h
 *
 * Created: 20.03.2024 12:05:28
 *  Author: zalto
 */ 


#ifndef REGULATOR_H_
#define REGULATOR_H_

typedef struct {
	float y0;              // Target value
	float y;               // Measured value
	float y_last;
	float error_integral;  // Integral error
	float error_last;      // Last error
	float error;
	float kp;              // Proportional constant
	float ki;              // Integral constant
	float kd;              // Derivative constant
	float u;
	float p;
	float i;
	float d;
} PIDregulator;

void pid_init(PIDregulator *pid, float y0, float y, float kp, float ki, float kd);
void pid_regulator(PIDregulator *pid);



#endif /* REGULATOR_H_ */