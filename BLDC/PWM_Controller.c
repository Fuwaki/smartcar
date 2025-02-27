#include <AI8051U.H>
#include <intrins.h>

#define FOSC 12000000L
#define PWM_FREQ 50
#define PWM_CYCLE 100
#define TIMER_RELOAD (256 - ((FOSC/12) / (PWM_FREQ * PWM_CYCLE))) //pretty well, isn't it?

unsigned char PWM_DUTY0;
unsigned char PWM_DUTY1;
unsigned char PWM_DUTY2;
unsigned char PWM_DUTY3;

unsigned char PWM_EN0;
unsigned char PWM_EN1;
unsigned char PWM_EN2;
unsigned char PWM_EN3;

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

    if (pwm_count < PWM_DUTY0)
    {
        PWM_EN0 = 1;
    }
    else
    {
        PWM_EN0 = 0;
    }

    if (pwm_count < PWM_DUTY1)
    {
        PWM_EN1 = 1;
    }
    else
    {
        PWM_EN1 = 0;
    }

    if (pwm_count < PWM_DUTY2)
    {
        PWM_EN2 = 1;
    }
    else
    {
        PWM_EN2 = 0;
    }

    if (pwm_count < PWM_DUTY3)
    {
        PWM_EN3 = 1;
    }
    else
    {
        PWM_EN3 = 0;
    }
}

void Set_PWM_Duty(unsigned char num, unsigned char duty)
{
	switch(num)
	{
		case 0:
			PWM_DUTY0 = duty;
			break;
		case 1:
			PWM_DUTY1 = duty;
			break;
		case 2:
			PWM_DUTY2 = duty;
			break;
		case 3:
			PWM_DUTY3 = duty;
			break;
	}
}