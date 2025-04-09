#include <STC32G.H>
#include <intrins.h>
#include "AR_PF.h"
#include "Muti_SPI_Device.h"
#include "uart.h"
#include "Track.h"


sbit fk = P1^5;
//QWQing
// enum State{
//     INIT=0,
//     RUNNING1,           //惯性导航主导的寻迹
//     RUNNING2,           //视觉导航主导的寻迹
//     SENSOR_FAULT,
//     CAMERA_FAULT,
//     TRACKING_FAULT,
//     STOPPING,
//     STOPPED,
// };
// enum State state = INIT;
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

    Uart3Init();
    ES3 = 1;
    EA = 1;
}
// void Change_State(enum State new_state){
//     switch(state){
//         case INIT:
//         Init();
//         break;
//         case RUNNING1:
//         break;
//         case RUNNING2:
//         case SENSOR_FAULT:
//         break;
//         case CAMERA_FAULT:
//         break;
//         case TRACKING_FAULT:
//         break;
//         case STOPPED:
//         break;
//     }
//     state = new_state;
// }
// void Running(){
//     switch(state){
//         case INIT:
//         //给传感器和摄像头发唤起命令
//         //等待传感器和摄像头初始化完成 等待贝塞尔曲线节点
//         break;
//         case RUNNING1:{
//             struct Boat_State bs=Track_Update();
//             struct Motor_State ms=Track_Control(bs);
//             Motor_Apply_State(ms);
//             break;
//         }
//         case RUNNING2:
//         //获取ccd误差
//         break;
//         case SENSOR_FAULT:
//         //状态显示
//         Change_State(STOPPING);
//         break;
//         case CAMERA_FAULT:
//         //状态显示
//         Change_State(STOPPING);
//         break;
//         case TRACKING_FAULT:
//         //状态显示
//         Change_State(STOPPING);
//         break;
//         case STOPPING:

//         break;
//         case STOPPED:

//         break;
//     }

// }
void Delay100ms(void)	//@35.000MHz
{
	unsigned long edata i;

	_nop_();
	_nop_();
	i = 874998UL;
	while (i) i--;
}

void Delay100us(void)	//@35.000MHz
{
	unsigned long edata i;

	_nop_();
	_nop_();
	_nop_();
	i = 873UL;
	while (i) i--;
}



void main()
{
    Init();
	fk = 1;
    while (1)
    {
        // Running();
		fk =~ fk;
		Delay100us();

    }
}