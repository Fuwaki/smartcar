#include <STC32G.H>
#include <intrins.h>
#include "PWM_Controller.h"
#include "FOC_Controller.h"
#include "PID_Controller.h"
#include "Lowpass_Filter.h"
#include "Observer.h"
#include "ADC.h"

void Delay100us(void)
{
	unsigned long edata i;

	_nop_();
	_nop_();
	_nop_();
	i = 873UL;
	while (i) i--;
}

void Inits()
{
	EAXFR = 1; // 使能访问 XFR
	CKCON = 0x00; // 设置外部数据总线速度为最快
	WTST = 0x00; // 设置程序代码等待参数，等待时间为0个时钟，CPU执行程序速度最快

	P1M0 = 0xff; P1M1 = 0x00; 

	// P1M1 = 0x00;P1M0 = 0x00;		//设置P1口为准双向口模式 //00：准双向口 01：推挽输出 10：高阻输入 11：开漏输出
	P2M1 = 0x00;P2M0 = 0xFF;		//设置P2口为准双向口模式 //00：准双向口 01：推挽输出 10：高阻输入 11：开漏输出
	P3M1 = 0x00;P3M0 = 0x00;		//设置P3口为准双向口模式 //00：准双向口 01：推挽输出 10：高阻输入 11：开漏输出
	P4M1 = 0x00;P4M0 = 0x00;		//设置P4口为准双向口模式 //00：准双向口 01：推挽输出 10：高阻输入 11：开漏输出
	P5M1 = 0x00;P5M0 = 0xFF;		//设置P5口为准双向口模式 //00：准双向口 01：推挽输出 10：高阻输入 11：开漏输出
	P6M1 = 0x00;P6M0 = 0x00;		//设置P6口为准双向口模式 //00：准双向口 01：推挽输出 10：高阻输入 11：开漏输出
	P7M1 = 0x00;P7M0 = 0x00;		//设置P7口为准双向口模式 //00：准双向口 01：推挽输出 10：高阻输入 11：开漏输出

	ADC_Init();
    PWM_Init();
	Timer0_Init();
	// Timer1_Init();
}

void main()
{
	Inits();
	Delay100us();
	// velocityOpenloop(50);
	// Delay100us();
	// Update_Observer(0,0,0,0);
	// getMotorInitAngle();
	while(1)
	{
		velocityOpenloop(.001f);
	}
}
