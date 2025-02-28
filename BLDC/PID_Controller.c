#include <AI8051U.H>
#include <intrins.h>
#include "PWM_Controller.h"

float kp;
float ki;
float kd;
float error;
float error_last;

double PID_Controller(double setpoint, double actual_value)
{
    double pid;
    double integral;
    double derivative;
    
    error = setpoint - actual_value;
    integral += error;
    derivative = error - error_last;
    pid = kp * error + ki * integral + kd * derivative;
    
    error_last = error;

    
    float dt = timestamp - timestamp_previous;
    if (dt <= 0)
    {
        dt = 0.001;
    }

    
    return pid;
}