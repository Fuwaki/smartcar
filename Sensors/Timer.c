#include <AI8051U.H>

float timestamp = 0;

void Timer0_Init(void) // 100us的定时器
{
    TMOD |= 0x01;
    TH0 = 0xF2;
    TL0 = 0xA4;
    ET0 = 1;
    EA = 1;
    TR0 = 1;
}

void Timer1_Init(void) // 100us的定时器
{
    TMOD |= 0x10;
    TH1 = 0xF2;
    TL1 = 0xA4;
    ET1 = 1;
    TR1 = 1;
}

void Timer0_ISR(void) interrupt 1
{
    TH0 = 0xF2;
    TL0 = 0xA4;
    
    timestamp = timestamp + 0.0001;
}