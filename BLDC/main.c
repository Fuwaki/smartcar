#include <STC32G.H>
#include <intrins.h>
#include "PWM_Controller.h"
#include "FOC_Controller.h"
#include "PID_Controller.h"
#include "Lowpass_Filter.h"
#include "Observer.h"
#include "uart_Debug.h"
#include "AR_PF.h"
#include "ADC.h"

float focData[3]; // 修改为float类型，与VOFA_SendFloats函数参数匹配

void Delay100us(void)
{
	unsigned long edata i;

	_nop_();
	_nop_();
	_nop_();
	i = 873UL;
	while (i) i--;
}

void Delay1000ms(void)	//@35.000MHz
{
	unsigned long edata i;

	_nop_();
	_nop_();
	i = 8749998UL;
	while (i) i--;
}


void Inits()
{
	EAXFR = 1; // 使能访问 XFR
	CKCON = 0x00; // 设置外部数据总线速度为最快
	// P_SW2 |= 0x80;       // 解锁特殊功能寄存器
	WTST = 0x00; // 设置程序代码等待参数，等待时间为0个时钟，CPU执行程序速度最快

    P1M0 = 0xff; P1M1 = 0x00; 

	// 特别注意：修改P3口模式配置，保证P3.1(TX)为推挽输出
	P2M1 = 0x00; P2M0 = 0xFF;
	P3M1 = 0x00; P3M0 = 0x02;
	P4M1 = 0x00; P4M0 = 0x00;
    P5M0 = 0x10; P5M1 = 0x10;
	P6M1 = 0x00;P6M0 = 0x00;
	P7M1 = 0x00;P7M0 = 0x00;

	ADC_Init();
    PWM_Init();
	Timer0_Init();
    Uart3Init();
	// Timer1_Init();
}

void main()
{
    Inits();
    Delay100us();

    while(1)
    {
		focData[0] = (float)Ua; // 添加显式类型转换
		focData[1] = (float)Ub; // 添加显式类型转换
		focData[2] = (float)Uc; // 添加显式类型转换
		VOFA_SendFloats(focData, 3);

        velocityOpenloop(.001f);
		Delay1000ms();
    }
    
}
