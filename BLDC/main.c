#include <AI8051U.H>
#include <intrins.h>
#include "PWM_Controller.h"
#include "FOC_Controller.h"

sbit PWMa = P0^0;
sbit PWMb = P0^1;
sbit PWMc = P0^2;

void Delay100us(void)	//@12.000MHz
{
	unsigned long edata i;

	_nop_();
	_nop_();
	_nop_();
	i = 298UL;
	while (i) i--;
}

void Inits()
{
	P0M0 = 0xff; P0M1 = 0x00;
	PWM_Init();
	Delay100us(); 
}

// typedef (int)
// void run(void * callback)
// {
	
// }

void En_PWM()
{
	PWMa = PWM_EN0;
	PWMb = PWM_EN1;
	PWMc = PWM_EN2;
}

void main (void)
{
	Inits();

	while(1)
	{
		velocityOpenloop(2.0);
	}
}
