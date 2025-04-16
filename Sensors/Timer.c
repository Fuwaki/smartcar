#include <AI8051U.H>
#include "Gyroscope.h"
#include "GPS.h"

#define YAW_ADJUST_VALUE .5f
float timestamp = 0;

void Timer0_Init(void)		//1毫秒@35.000MHz
{
	AUXR |= 0x80;			//定时器时钟1T模式
	TMOD &= 0xF0;			//设置定时器模式
	TL0 = 0x48;				//设置定时初始值
	TH0 = 0x77;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
}

void TRUE_YAW_GET()
{
    if (!allowUpdate) // 如果不允许更新航向角数据，则返回
        return;

    // 计算真实航向角
    gyro_data.true_yaw_angle += gyro_data.gyro_z_dps * 0.001f; // 更新真实航向角
    if (gyro_data.true_yaw_angle >= 360.0f) gyro_data.true_yaw_angle = 0.0f; // 限制在0到360度之间
    if (gyro_data.true_yaw_angle < 0.0f) gyro_data.true_yaw_angle = 360.0f; // 限制在0到360度之间

    //GPS纠正航向角 
    //能运行到这里gps不应该是无效的吧qwq
    if (gyro_data.true_yaw_angle > rmc_data.course)
    {
        gyro_data.true_yaw_angle += YAW_ADJUST_VALUE;
    }
    else if (gyro_data.true_yaw_angle < rmc_data.course)
    {
        gyro_data.true_yaw_angle -= YAW_ADJUST_VALUE;
    }

}

void Timer0_ISR(void) interrupt 1
{
	TL0 = 0xD8;				//设置定时初始值
	TH0 = 0xFF;				//设置定时初始值
    timestamp = timestamp + 0.001f;
    True_YAW_GET(); // 更新真实航向角
}