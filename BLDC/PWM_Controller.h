#ifndef __PWM_CONTROLLER_H__
#define __PWM_CONTROLLER_H__
    extern float timestamp;
    extern float timestamp_previous;
    extern float dt;
    void PWM_Init();
    void Set_PWM_Duty(unsigned char channel, unsigned char duty);
    void Timer0_Init();
    void Timer1_Init();
#endif