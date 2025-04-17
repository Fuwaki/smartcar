#include <STC32G.H>
#include <intrins.h>
#include "main.h"

#define VALUE_FOR_SWITCH 10000 //开关值
unsigned long timestamp = 0; //单位是100us(0.1ms为1)

sbit SW1 = P5 ^ 2;
sbit SW2 = P0 ^ 4;
sbit SW3 = P0 ^ 3;
sbit SW4 = P0 ^ 2;
sbit SW5 = P4 ^ 6;
sbit SW6 = P4 ^ 5;

bit SelfFuckerBit = 0; //自毁开关

void Timer0_Init(void)	//100us@35.000MHz
{
    AUXR |= 0x80;			//定时器0为1T模式
    TMOD &= 0xF0;			//清除定时器0模式位
    TMOD |= 0x01;			//定时器0为模式1(16位定时)
	TL0 = 0x54;				//设置定时初始值
	TH0 = 0xF2;				//设置定时初始值
    TF0 = 0;				//清除TF0标志
    TR0 = 1;				//启动定时器0
    ET0 = 1;				//使能定时器0中断
    EA = 1;					//使能总中断
}

void SwitchUpdater()
{
    if (SW1 == 0)
    {
        Start(); //启动函数
    }
    else if (SW2 == 0)
    {
        AddCurrentPositionAsPathPoint(); //添加当前点到路径点
    }
    else if (SW3 == 0)
    {
        AddCurrentPositionAsStopPoint(); //添加当前点到停止点
    }
    else if (SW4 == 0)
    {
        Stop(); //停止函数
    }
    else if (SW5 == 0)
    {
        ClearPath(); //清除路径函数
    }
    else if (SW6 == 0)
    {
        //playSound(CIALLO!);
    }

    if (SW1 == 0 || SW2 == 0 || SW3 == 0 || SW4 == 0 || SW5 == 0 || SW6 == 0) //请不要当傻逼,同时按下多个开关
    {
        SelfFuckerBit = 1; //自毁开关
    }
}

void Timer0_ISR(void) interrupt 1
{
    static unsigned long count = 0; // 计数器
    timestamp++; // 每次中断增加0.1ms
    SwitchUpdater(); // 检测开关状态

    if (SelfFuckerBit == 1) // 自毁开关被按下
    {
        count++;
        if (count >= VALUE_FOR_SWITCH) // 达到开关值
        {
            SelfFuckerBit = 0; // 重置自毁开关
            count = 0; // 重置计数器
        }
    }
}