#include <STC32G.H>
#include <intrins.h>
#include "PWM_Controller.h"
#include "FOC_Controller.h"
#include "PID_Controller.h"
#include "Lowpass_Filter.h"
#include "Observer.h"
#include "uart_Debug.h"
#include "TrigTable.h"
#include "AR_PF.h"
#include "ADC.h"

#define speed 1.04 // 速度单位为rad/s

sbit LED = P4^2;
sbit LED2 = P4^1;

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

	P0M0 = 0x00; P0M1 = 0x00;
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
	initTrigTables(); // 初始化三角函数表
	ES3 = 1;
	// Timer1_Init();
}

void main()
{
	double i = 0;
	int a= 0;


    Inits();
    P4M0 |= 0x06; P4M1 &= ~0x06; 

    // Delay100us();
	LED2 = 1;
	LED = 1;
	// Uart3SendStr("Hello World!\0");
    while(1)
    {
		focData[0] = (float)Ua; // 添加显式类型转换
		focData[1] = (float)Ub; // 添加显式类型转换
		focData[2] = (float)Uc;
		VOFA_SendFloat(focData); // 修改为float类型，与focData数组匹配
		LED2 =~	LED2;
		if(i<speed)
		{
			a++;
			if (a%30==0&&i<speed)
			{
				i+=0.001;
			}
			velocityOpenloop(i);
		}
		else
		{
			velocityOpenloop(speed);
			LED = 0;
		}

		// Delay1000ms();
		// Delay100us();
    }
    
}
