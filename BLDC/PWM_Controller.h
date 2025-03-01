#ifndef __PWM_CONTROLLER_H__
#define __PWM_CONTROLLER_H__
    extern unsigned char PWM_DUTY0;
    extern unsigned char PWM_DUTY1;
    extern unsigned char PWM_DUTY2;
    extern unsigned char PWM_DUTY3;
    extern double timestamp;
    extern double timestamp_previous;
    void PWM_Init(unsigned int freq, unsigned char dead_time);
    void Set_PWM_Frequency(unsigned int freq);
    void Set_PWM_Duty(unsigned char num, unsigned char duty);
    void PWM_Channel_Controller(unsigned char channel, unsigned char enable);
    void Timer0_Init();
    void Timer1_Init();
#endif