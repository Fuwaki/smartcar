#include <AI8051U.H>

float timestamp = 0;

void Timer0_Init(void) // 1ms的定时器
{
    TMOD |= 0x01;
	TL0 = 0xD8;				//设置定时初始值
	TH0 = 0xFF;				//设置定时初始值
    ET0 = 1;
    EA = 1;
    TR0 = 1;
}

void Timer0_ISR(void) interrupt 1
{
	TL0 = 0xD8;				//设置定时初始值
	TH0 = 0xFF;				//设置定时初始值
    
    timestamp = timestamp + 0.01f;
}