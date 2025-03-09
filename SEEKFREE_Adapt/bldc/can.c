#include "can.h"
#include <common.h>
typedef unsigned char u8;
typedef unsigned int u16;
u8 rx_buf[8];
u8 tx_buf[8];
//标准帧的信息长度都是8
void can_init()
{
    // TODO: 设置波特率

    // 切换引脚到p00和p01
    CAN_S1 = 0;
    CAN_S0 = 0;
    CANICR = 0x02; // 启用CAN中断
    CANEN = 1;     // 启用CAN模块
}
// 使用标准帧就好
void can_interrupt() interrupt 28
{
    u8 isr;
    isr = can_get_reg(ISR);
    if ((isr) & (1 << 2) == (1 << 2))
    {
        // 发送完成中断
        CANAR = 0x03;
        CANDR = 0x04;
    }
    if ((isr) & (1 << 3) == (1 << 3))
    {
        // 信息接受中断
        


    }
}
// can的所有寄存器需要间接获取
void can_set_reg(u8 addr, u8 dat)
{
    CANAR = addr;
    CANDR = dat;
}
u8 can_get_reg(u8 addr)
{
    char dat;
    CANAR = addr;
    dat = CANDR;
    return dat;
}
void can_send_msg()
{
}
void can_recv_msg(){

}
// 关于遥控帧
/*
01 设置转速
00 获得转速
10 获得电流
*/