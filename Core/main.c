#include "../library/can.h"
#include "AR_PF.h"
#include "Error.h"
#include "Motor.h"
#include "Oled.h"
#include "Track.h"
#include "i2c.h"
#include "uart.h"
#include <STC32G.H>
#include <intrins.h>
#include <math.h>
// 这么写是为了后面布局各种周期不同的任务
bit shouldUpdateControl = 0;

sbit led = P4 ^ 0;

int flag = 0; // 0待机 1控制闭环 -1过渡态
typedef void (*TransitionUpdateFunc)();
TransitionUpdateFunc TransitionUpdate = 0;
float launch_progress = 0.0;

// 抬升电机达到制定转速 其他电机起码要启动起来 怠速状态
#define TARGET_LIFTING_SPEED 0.7
#define TARGET_IDLE_SPEED 0.1
#define LAUNCH_SPEED 0.001 // 每次进度推进多少

void MildLaunch()
{
    struct Motor_State motor_state;
    if (launch_progress < 0.0)
    {
        // ERROR(2, "launch_progress out of range");
        return;
    }
    if (launch_progress > 1.0)
    {
        flag = 1;           //启动完成
        TransitionUpdate = 0; // 过渡完成
        return;
    }
    motor_state.bottom_right = motor_state.bottom_left = TARGET_LIFTING_SPEED * launch_progress;
    motor_state.back_left = motor_state.back_right = TARGET_IDLE_SPEED * launch_progress;

    Motor_Apply_State(motor_state);
    launch_progress += LAUNCH_SPEED;
    //100微秒延时
    {
	unsigned long edata i;

	_nop_();
	_nop_();
	_nop_();
	i = 873UL;
	while (i) i--;
    }

}

// 响应按钮实现的函数
void Start()
{
    launch_progress=0.2;
    TransitionUpdate = MildLaunch;
    flag = -1;
}
void AddCurrentPositionAsPathPoint()
{


}
void AddCurrentPositionAsStopPoint()
{
}
void Stop()
{
    struct Motor_State motor_state;
    motor_state.bottom_right = 0.0f;
    motor_state.bottom_left = 0.0f;
    motor_state.back_left = 0.0f;
    motor_state.back_right = 0.0f;
    Motor_Apply_State(motor_state);
    flag = 0;
}
void ClearPath()
{

}

void Init()
{
    EAXFR = 1;    // 使能访问 XFR,没有冲突不用关闭
    CKCON = 0x00; // 设置外部数据总线速度为最快
    WTST = 0x00;  // 设置程序代码等待参数，
    // 赋值为 0 可将 CPU 执行程序的速度设置为最快
    P0M0 = 0x00;
    P0M1 = 0x00;
    P1M0 = 0x00;
    P1M1 = 0x00;
    P2M0 = 0x00;
    P2M1 = 0x00;
    P3M0 = 0x00;
    P3M1 = 0x00;
    P4M0 = 0x00;
    P4M1 = 0x00;
    P5M0 = 0x00;
    P5M1 = 0x00;

    Uart3Init();
    Uart1Init();
    I2C_Init();    // 初始化I2C
    OLED_Init();   // 初始化OLED
    Timer2_Init(); // 初始化定时器2
    // Motor_Init();  // 初始化电机
    EA = 1;
}
// 检测状态是否需要切换
void StatusSwitch()
{
    SwitchUpdater(); // 检测开关状态并更新显示
}

// 传感器数据更新
struct Posture SensorUpdate()
{
    // TODO:在这里加入一些传感器错误处理
    struct Posture posture;
    UartReceiveSensorData();

    posture.position[0] = sensor_data.GPS_Raw_X; // 纬度
    posture.position[1] = sensor_data.GPS_Raw_Y; // 经度

    posture.attitude[0] = 0;                       // 俯仰角
    posture.attitude[1] = 0;                       // 横滚角
    posture.attitude[2] = sensor_data.IMU_Heading; // 方向

    posture.angular_velocity[0] = sensor_data.IMU_Acc_X; //
    posture.angular_velocity[1] = sensor_data.IMU_Acc_Y; //
    posture.angular_velocity[2] = sensor_data.IMU_Acc_Z; //
}

// 控制函数执行超时
void OnControlUpdateTimeout()
{
    // TODO：当控制函数超过1s没有被调用 那么触发这个函数 触发停机保护
}
void ControlUpdate()
{
    if (shouldUpdateControl)
    {
        struct Posture posture;
        struct BoatState boat_state;
        struct Motor_State motor_state;

        posture=SensorUpdate();
        boat_state = Track_Update(posture,1.0);         //FIXME:使用定时器来获得准确的dt
        motor_state = Track_ToMotorState(boat_state);
        Motor_Apply_State(motor_state);

        shouldUpdateControl = 0;
    }
}

void IdleUpdate()
{
    //不空转还能干嘛
    //那我问你
    //LOOK IN MY EYES
}
void Run()
{
    // TODO:看情况加延时函数
    switch (flag)
    {
    case -1:
        if (TransitionUpdate != 0)
        {
            TransitionUpdate();
        }
        else
        {
            ERROR(1, "TransitionUpdate is NULL");
        }
    case 1:
        // ControlUpdate();
        break;
    case 0:
        // IdleUpdate();
        break;
    default:
        break;
    }
}

void main()
{

    
    Init();
    ES = 1; // 使能串口中断
    led = 1;
    
    while (1)
    {

        led = ~led;     // 反转LED灯

        if (timestamp == floor(timestamp))
            shouldUpdateControl = 1; // 1ms更新一次控制函数

        if (error_flag)
        {
            // 响应ERROR.c中收到的错误
            // error_msg是字符串 看看要不要输出到oled
            P33=0;
        }
        else
        {
            // 正常运行
            // StatusSwitch();
            Run();
        }
    }
}