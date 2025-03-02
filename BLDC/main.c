#include <STC32G.H>
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
    PWM_Init(20000, 16); //20KHz PWM频率, 16个系统时钟周期的死区时间
	Timer0_Init();
	// Timer1_Init();
	Delay100us(); 
}

void main (void)

{
	Inits();
    // 设置各通道占空比
    Set_PWM_Duty(1, 50);  // 通道1: 50%占空比
    Set_PWM_Duty(2, 30);  // 通道2: 30%占空比
    Set_PWM_Duty(3, 70);  // 通道3: 70%占空比
    
    // 单独控制通道
    PWM_Channel_Controller(1, 1);  // 使能通道1
    PWM_Channel_Controller(2, 1);  // 使能通道2
    PWM_Channel_Controller(3, 1);  // 使能通道3
    
	while(1)
	{
		velocityOpenloop(5);
	}
}
