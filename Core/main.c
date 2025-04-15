#include <STC32G.H>
#include <intrins.h>
#include "../library/can.h"
#include "AR_PF.h"
#include "uart.h"
#include "Track.h"
#include "Motor.h"
#include "i2c.h"
#include "Oled.h"
#include "Error.h"
int flag=0;
//这么写是为了后面布局各种周期不同的任务
bit shouldUpdateControl=0;


void Init(){
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
    P2M0 |= 0x80; P2M1 &= ~0x80; 

    // Uart3Init();
    Uart1Init();
    I2C_Init(); // 初始化I2C
    EA = 1;
}
//检测状态是否需要切换
void StatusSwitch(){

}
//传感器数据更新
struct Posture SensorUpdate(){
    
    UartReceiveSensorData();
}

//控制函数执行超时
void OnControlUpdateTimeout(){
    //TODO：当控制函数超过1s没有被调用 那么触发这个函数 触发停机保护
}
void ControlUpdate(){
    if(shouldUpdateControl){
        struct Posture posture;
        struct BoatState boat_state ;
        struct Motor_State motor_state;

        posture=SensorUpdate();
        boat_state = Track_Update(posture);
        motor_state= Track_Control(boat_state);
        Motor_Apply_State(motor_state);

        shouldUpdateControl=0;
    }
}
void DebugUpdate(){
    //按钮调参+屏幕显示
}

void Run(){
    switch(flag){
        case 0:
            ControlUpdate();
            break;
        case 1:
            DebugUpdate();
            break;
        case 2:
            // Track_Update();
            break;
        default:
            break;
    }
}



void main()
{
    Init();
    ES = 1; // 使能串口中断
    while (1)
    {
        if(error_flag){
            //响应ERROR.c中收到的错误
            //error_msg是字符串 看看要不要输出到oled
        }else{
            //正常运行
            StatusSwitch();
            Run();
        }
    }
}