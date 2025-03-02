
#ifndef _sine_control_h
#define _sine_control_h
#include "common.h"




extern int16 motor_a_position;
extern int16 motor_b_position;
extern int16 motor_c_position;


void sine_init(void);
void sine_start(uint8 duty);

#endif
