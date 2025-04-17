#ifndef __MOTOR_H
#define __MOTOR_H
//所有控制在0-1之间
//TODO: 之后看能不能考虑反着转
struct Motor_State{
    float left,right;
    float back_left,back_right; 
    float bottom_left,bottom_right;
};
void Motor_Apply_State(struct Motor_State state);
void Motor_Init();
#endif