#include <AI8051U.H>
#include <intrins.h>
#include "PWM_Controller.h"

#define FOSC 12000000L
#define PWM_FREQ 50
#define PWM_CYCLE 100
#define TIMER_RELOAD (256 - ((FOSC/12) / (PWM_FREQ * PWM_CYCLE))) //pretty well, isn't it?

// PWM占空比
unsigned char DEADTIME = 1;
unsigned char PWM_DUTY0;
unsigned char PWM_DUTY1;
unsigned char PWM_DUTY2;

// PWM输出引脚控制变量 (H=高边，L=低边)
unsigned char PWM_H0;
unsigned char PWM_L0;
unsigned char PWM_H1;
unsigned char PWM_L1;
unsigned char PWM_H2;
unsigned char PWM_L2;

static unsigned char pwm_count = 0;
double timestamp = 0; //时间戳
double timestamp_previous = 0; //上一个时间戳

void PWM_Init(void)
{
    // 初始化占空比为0
    PWM_DUTY0 = 0;
    PWM_DUTY1 = 0;
    PWM_DUTY2 = 0;
    
    // 初始化PWM输出为关闭状态
    PWM_H0 = 0;
    PWM_L0 = 0;
    PWM_H1 = 0;
    PWM_L1 = 0;
    PWM_H2 = 0;
    PWM_L2 = 0;
    
    // 配置定时器0为模式2 (8位自动重载)
    TMOD &= 0xF0;
    TMOD |= 0x02;
    TH0 = TIMER_RELOAD;
    TL0 = TIMER_RELOAD;
    ET0 = 1;  // 启用定时器0中断
    TR0 = 1;  // 启动定时器0
    EA = 1;   // 全局中断使能
}

void Timer0_ISR(void) interrupt 1
{
    pwm_count++;
    timestamp_previous = timestamp;
    timestamp += 0.001;
    
    if(pwm_count >= PWM_CYCLE)
    {
        pwm_count = 0;
    }
    
    // 通道0互补PWM控制
    if(pwm_count < PWM_DUTY0)
    {
        PWM_H0 = 1;
        PWM_L0 = 0;
    }
    else if(pwm_count == PWM_DUTY0) 
    {
        PWM_H0 = 0;
    }
    else if(pwm_count >= (PWM_DUTY0 + DEADTIME))
    {
        PWM_L0 = 1;
    }
    
    // 通道1互补PWM控制
    if(pwm_count < PWM_DUTY1)
    {
        PWM_H1 = 1;
        PWM_L1 = 0;
    }
    else if(pwm_count == PWM_DUTY1)
    {
        PWM_H1 = 0;
    }
    else if(pwm_count >= (PWM_DUTY1 + DEADTIME))
    {
        PWM_L1 = 1;
    }
    
    // 通道2互补PWM控制
    if(pwm_count < PWM_DUTY2)
    {
        PWM_H2 = 1;
        PWM_L2 = 0;
    }
    else if(pwm_count == PWM_DUTY2)
    {
        PWM_H2 = 0;
    }
    else if(pwm_count >= (PWM_DUTY2 + DEADTIME))
    {
        PWM_L2 = 1;
    }
    
    // PWM周期开始时重置低边
    if(pwm_count == 0)
    {
        PWM_L0 = 0;
        PWM_L1 = 0;
        PWM_L2 = 0;
    }
}

void Set_PWM_Duty(unsigned char num, unsigned char duty)
{
    // 限制占空比范围
    if(duty > (PWM_CYCLE - DEADTIME - 1))
        duty = PWM_CYCLE - DEADTIME - 1;
    
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
    }
}