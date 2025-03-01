#include "PWM_Controller.h"
#define _constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

double error = 0;
double error_prev = 0;

float kp = 0.1;
float ki = 0.1;
float kd = 0.1;
double max_pid;
double min_pid;
double integralMin;
double integralMax;

void AdjustPID(float kp_, float ki_, float kd_)//调整PID参数 但是没想好
{
    kp = kp_;
    ki = ki_;
    kd = kd_;
}

double PID_Controller(double setpoint, double actual_value)
{
    // double dt;
    // dt = timestamp - timestamp_previous;

    double pid = 0;
    double integral = 0;
    double derivative = 0;

    error = setpoint - actual_value;
    integral = _constrain(integral + error, integralMin, integralMax);
    derivative = error - error_prev;
    pid = _constrain(kp * error + ki * integral + kd * derivative, min_pid, max_pid);
    
    error_prev = error;
    return pid;
}