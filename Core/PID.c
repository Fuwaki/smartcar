#include "PID.h"
void PID_Init(PID_Control *pc, float kp, float ki, float kd)
{
    pc->kp = kp;
    pc->ki = ki;
    pc->kd = kd;
    pc->last_error = 0;
    pc->error_sum = 0;
    pc->integral_limit = 0;
    pc->output_limit = 0;
}

void PID_Set_Integral_Limit(PID_Control *pc, float limit){
    pc->integral_limit = limit;
}
void PID_Set_Output_Limit(PID_Control *pc, float limit){
    pc->output_limit = limit;
}
float PID_Update(PID_Control *pc, float error,float dt)
{
    float p,i,d;
    p = pc->kp * error;
    pc->error_sum += error * dt;
    if (pc->integral_limit != 0)
    {
        if (pc->error_sum > pc->integral_limit)
        {
            pc->error_sum = pc->integral_limit;
        }
        else if (pc->error_sum < -pc->integral_limit)
        {
            pc->error_sum = -pc->integral_limit;
        }
    }
    i = pc->ki * pc->error_sum;
    d = pc->kd * (error - pc->last_error) / dt;
    pc->last_error = error;
    return p + i + d;
}