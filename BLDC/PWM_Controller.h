#ifndef __PWM_CONTROLLER_H__
#define __PWM_CONTROLLER_H__
    extern float timestamp;
    extern float timestamp_previous;
    void PWM_Init(unsigned int freq, unsigned char dead_time);
    void Set_PWM_Frequency(unsigned int freq);
    void Set_PWM_Duty(unsigned char num, unsigned char duty);
    void PWM_Channel_Controller(unsigned char channel, unsigned char enable);
    void Set_PWM_DeadTime(unsigned char dead_time);
    void Timer0_Init();
    void Timer1_Init();
#endif