#include <AI8051U.H>
#include <intrins.h>

#define FOSC 12000000L
#define PWM_FREQ 50
#define PWM_CYCLE 100
#define TIMER_RELOAD (256 - ((FOSC/12) / (PWM_FREQ * PWM_CYCLE))) //pretty well, isn't it?

static unsigned char pwm_count = 0;

void PWM_Init(void) //use this if you want to FK the PWM controller
{
    TMOD &= 0xF0;
    TMOD |= 0x02;
    TH0 = TIMER_RELOAD;//??
    TL0 = TIMER_RELOAD;
    ET0 = 1;
    TR0 = 1;
    EA = 1;
}

typedef struct //没想好，上课去了lol
{
    unsigned char pwm;
    unsigned char dir;
} PWM_Controller;

void Timer0_ISR(void) interrupt 1
{
    pwm_count++;

    if(pwm_count >= PWM_CYCLE)
    {
        pwm_count = 0;
    }
}

