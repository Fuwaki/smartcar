#include <AI8051U.H>
#include <intrins.h>
#include "PWM_Controller.h"
#include "FOC_Controller.h"
#include "PID_Controller.h"
#include "Observer.h"
#include "Lowpass_Filter.h"

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
	P0M0 = 0xff; P0M1 = 0x00; //设置P0口为推挽输出
	P1M0 = 0xff; P1M1 = 0x00; //设置P1口为推挽输出 (PWM输出)
    PWM_Init(20000, 16); //20KHz PWM频率, 16个系统时钟周期的死区时间
	Timer0_Init();
	// Timer1_Init();
	Delay100us(); 
}

void main (void)
{
	Inits();

	while(1)
	{
		velocityOpenloop(5);
	}
}
