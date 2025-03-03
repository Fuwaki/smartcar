#include <STC32G.H>
#include "intrins.h"

typedef unsigned char u8;
typedef unsigned int u16;
typedef unsigned long int u32;

#define FOSC 35000000L  //系统时钟35MHz
#define PWM_PSC (35-1) //35分频，时钟周期1us
#define PWM_PERIOD 200 //周期200us
#define PWM_DUTY 100 //占空100us
#define _duty_Cal(value) ((u16)((PWM_PERIOD/100)*value)) //占空比计算
#define PWM_DTime 350 //死区10us
#define PWM_TIMER_PERIOD 200 

float timestamp = 0;
float timestamp_previous = 0;

void PWM_Init(void)
{
	PWMA_PSCRH = (u16)(PWM_PSC >> 8);		//预分频
	PWMA_PSCRL = (u16)(PWM_PSC);
	
	PWMA_CCER1 = 0x00; 									// 写 CCMRx 前必须先清零 CCERx 关闭通道
	PWMA_CCMR1 = 0x68; 									// 设置 CC1 CC1N为 PWMA 输出模式，PWM模式1
	PWMA_CCER1 = 0x05; 									// 使能 CC1 CC1N通道

    // ==== 第2对互补PWM配置(通道2) ====
    PWMA_CCER1 &= 0x0F;                       // 保留通道1配置，清除通道2配置
    PWMA_CCMR2 = 0x68;                        // 设置 CC2 CC2N为 PWMA 输出模式，PWM模式1
    PWMA_CCER1 |= 0x50;                       // 使能 CC2 CC2N通道(位置4和位置6)
    
    // ==== 第3对互补PWM配置(通道3) ====
    PWMA_CCER2 &= 0xF0;                       // 保留通道4配置，清除通道3配置
    PWMA_CCMR3 = 0x68;                        // 设置 CC3 CC3N为 PWMA 输出模式，PWM模式1
    PWMA_CCER2 |= 0x05;  
	
	PWMA_CCR1H = (u16)(PWM_DUTY >> 8);	// 设置初始占空
	PWMA_CCR1L = (u16)(PWM_DUTY); 

    PWMA_CCR2H = (u16)(PWM_DUTY >> 8);
    PWMA_CCR2L = (u16)(PWM_DUTY);

    PWMA_CCR3H = (u16)(PWM_DUTY >> 8);
    PWMA_CCR3L = (u16)(PWM_DUTY);
	
	PWMA_ARRH = (u16)(PWM_PERIOD >> 8); // 设置PWM周期
	PWMA_ARRL = (u16)(PWM_PERIOD); 
	
	PWMA_DTR = PWM_DTime;								// 插入死区时间
	
    PWMA_ENO = 0x3F;
	PWMA_BKR = 0x80;										// 使能主输出
	
	PWMA_CR1 = 0x01; 										// 启动PWMA定时器
}

/*num是通道名为0,1,2 || duty为0~100*/
void Set_PWM_Duty(unsigned char channel, unsigned char duty)
{
    if (duty > 100) //如果有某个入干了蠢事
    {
        duty = 100;
    }

    switch (channel)
    {
    case 0:
        PWMA_CCR1H = (u16)(_duty_Cal(duty) >> 8);
        PWMA_CCR1L = (u16)(_duty_Cal(duty));
        break;
    case 1:
        PWMA_CCR2H = (u16)(_duty_Cal(duty) >> 8);
        PWMA_CCR2L = (u16)(_duty_Cal(duty));
        break;
    case 2:
        PWMA_CCR3H = (u16)(_duty_Cal(duty) >> 8);
        PWMA_CCR3L = (u16)(_duty_Cal(duty));
        break;
    default:
        break;
    }
}

//定时器部分
void Timer0_Init()
{
    TMOD |= 0x01;
    TH0 = 0xFF;
    TL0 = 0x00; 
    ET0 = 1;
    EA = 1;
    TR0 = 1;
}

void Timer1_Init(void)
{
    TMOD |= 0x10;
    TH1 = 0xFF;
    TL1 = 0x00; 
    ET1 = 1;
    TR1 = 1;
}

void Timer0_ISR(void) interrupt 1
{
    static unsigned char pwmCounter1 = 0;
    TH0 = 0xFF;
    TL0 = 0x00;

    timestamp_previous = timestamp;
    timestamp = timestamp + 0.01;

    pwmCounter1++;
    if (pwmCounter1 >= PWM_TIMER_PERIOD)
    {
        pwmCounter1 = 0;
    }

    // if (pwmCounter1 < 50)
    // {
        /*put ur code here!*/
    // }
    // else
    // {
        /*put ur code here!*/
    // }
}

void Timer1_ISR(void) interrupt 3
{
    //place ur code when you want to use second timer!
}