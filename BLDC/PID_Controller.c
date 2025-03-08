#include "PWM_Controller.h"
#define _constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

double error_prev = 0;

float kp = 0.1;
float ki = 0.1;
float kd = 0.1;
double max_pid = 12/2; //power supply voltage/2
double min_pid = -12/2;
double integralMin = 10;
double integralMax = 10;
static double integral = 0;
static double derivative = 0;

void AdjustPID(float kp_, float ki_, float kd_)//调整PID参数 但是没想好
{
    kp = kp_;
    ki = ki_;
    kd = kd_;
}

double PID_Controller(double error) 
{
    double output = 0;

    integral = _constrain(integral + error, integralMin, integralMax); //积分限幅
    derivative = error - error_prev;
    output = _constrain(kp * error + ki * integral + kd * derivative, min_pid, max_pid); //输出限幅
    
    error_prev = error;
    return output;
}