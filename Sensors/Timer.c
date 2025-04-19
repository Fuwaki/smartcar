#include <AI8051U.H>
#include "Gyroscope.h"
#include "GPS.h"

#define YAW_ADJUST_VALUE .0005f
float timestamp = 0;

void Timer0_Init(void)		//1毫秒@35.000MHz
{
    AUXR |= 0x80;			//定时器0为1T模式
    TMOD &= 0xF0;			//清除定时器0模式位
    TMOD |= 0x01;			//定时器0为模式1(16位定时)
    TL0 = 0xC0;        // 设置定时初值，1ms定时（低字节）
    TH0 = 0x63;        // 设置定时初值（高字节）
    TF0 = 0;				//清除TF0标志
    TR0 = 1;				//启动定时器0
    ET0 = 1;				//使能定时器0中断
    EA = 1;					//使能总中断
}

void TRUE_YAW_GET()
{
    if (allowUpdate)
    {
        // 计算真实航向角
        gyro_data.true_yaw_angle += ((gyro_data.gyro_z_dps_kf * 0.001f) * 16.164f); // 更新真实航向角16.164610f是陀螺仪的灵敏度转换系数
        if (gyro_data.true_yaw_angle >= 360.0f) gyro_data.true_yaw_angle = 0.0f; // 限制在0到360度之间
        if (gyro_data.true_yaw_angle < 0.0f) gyro_data.true_yaw_angle = 360.0f; // 限制在0到360度之间
    }


    // //GPS纠正航向角 
    // //能运行到这里gps不应该是无效的吧qwq
    // if (rmc_data.valid == 1 && rmc_data.speed >= 1)
    // {
    //     if (gyro_data.true_yaw_angle > 360.0 - rmc_data.course)
    //     {
    //         gyro_data.true_yaw_angle += YAW_ADJUST_VALUE;
    //     }
    //     else if (gyro_data.true_yaw_angle < 360.0 - rmc_data.course)
    //     {
    //         gyro_data.true_yaw_angle -= YAW_ADJUST_VALUE;
    //     }
    // }

}

void Timer0_ISR(void) interrupt 1
{
    TL0 = 0xC0;        // 设置定时初值，1ms定时（低字节）
    TH0 = 0x63;        // 设置定时初值（高字节）
    timestamp = timestamp + 0.001f;
    True_YAW_GET(); // 更新真实航向角
}