#ifndef __PID_CONTROLLER_H__
#define __PID_CONTROLLER_H__
    double PID_Controller(double error);
    void AdjustPID(float kp_, float ki_, float kd_);
#endif