#ifndef __PWM_CONTROLLER_H__
#define __PWM_CONTROLLER_H__
    void PWM_Init(void);
    void Set_PWM_Duty(unsigned char num, unsigned char duty);
    // void Set_PWM_Enable(unsigned char num, unsigned char enable); //for later.
    extern unsigned char PWM_DUTY0;
    extern unsigned char PWM_DUTY1;
    extern unsigned char PWM_DUTY2;
    extern unsigned char PWM_DUTY3;
    extern unsigned char PWM_EN0;
    extern unsigned char PWM_EN1;
    extern unsigned char PWM_EN2;
    extern unsigned char PWM_EN3;
#endif