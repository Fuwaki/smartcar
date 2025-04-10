#include "PID.h"
void PID_Init(PID_Control *pc, float kp, float ki, float kd){
    pc->kp = kp;
    pc->ki = ki;
    pc->kd = kd;
    

}
float PID_Update(PID_Control* pc,float error){

    
}