#include "can.h"
#include <common.h>

typedef unsigned char u8;
typedef unsigned int u16;

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
enum CAN_FRAME_TYPE
{
    CAN_DATA_FRAME = 0,
    CAN_REMOTE_FRAME = 1,
};
//mask为1代表着不care
void can_set_filter(unsigned int cr,unsigned int mask){
    can_set_reg(ACR0,(u8)(cr>>3));
    can_set_reg(ACR1,(u8)(cr<<5));
    can_set_reg(AMR0,(u8)(mask>>3));
    can_set_reg(AMR1,(u8)(mask<<5)|0x1f);
    can_set_reg(ACR2,0);
    can_set_reg(ACR3,0);
    can_set_reg(AMR2,0xff);
    can_set_reg(AMR3,0xff);
}
void can_read_rx_fifo(u8 *array)
{
    array[0] = can_get_reg(RX_BUF0);
    array[1] = can_get_reg(RX_BUF1);
    array[2] = can_get_reg(RX_BUF2);
    array[3] = can_get_reg(RX_BUF3);
}
// stc 生成的设置波特率
void can_set_baudrate(void) // 125Kbps@30MHz
{
    can_set_reg(MR, 0x04);   // 使能Reset模式
    can_set_reg(BTR0, 0x04); // SJW(0), BRP(4)
    can_set_reg(BTR1, 0x6f); // SAM(0), TSG2(6), TSG1(15)
    can_set_reg(MR, 0x00);   // 退出Reset模式
}
// 发送封装好的数据包 长度一定要是4的倍数
void can_send_pack(u8 *dat, u8 len)
{
    unsigned short i;
    for (i = 0; i <= len % 4; i++)
    {
        can_set_reg(TX_BUF0, dat[i + 0]);
        can_set_reg(TX_BUF1, dat[i + 1]);
        can_set_reg(TX_BUF2, dat[i + 2]);
        can_set_reg(TX_BUF3, dat[i + 3]);
    }
    can_set_reg(CMR, 0x04); // 发起发送请求
}
// 标准帧的信息长度都是8
void can_init()
{
    CANSEL=0;       //选择第一组can
    // TODO: 设置波特率
    can_set_baudrate();

    // 切换引脚到p00和p01
    CAN_S1 = 0;
    CAN_S0 = 0;
    CANICR = 0x02; // 启用CAN中断
    CANEN = 1;     // 启用CAN模块
    
}
// 返回的size就是数据的size 不包含头 请保证dat足够大
enum CAN_FRAME_TYPE can_recv_msg(u8 *dat, u8 *size)
{
    u8 len = 0;
    short i;
    can_read_rx_fifo(dat);
    len = (dat[0] & 0xf) - 1; // 我只要低端的四位
    *size = len + 1;
    for (i = 0; i < len + 4; i += 4)
    {
        can_read_rx_fifo(dat + i);
    }
    return dat[0] & (1 << 6) ? CAN_REMOTE_FRAME : CAN_DATA_FRAME;
}
// 使用标准帧就好
void can_interrupt() interrupt 28
{
    u8 isr;
    isr = can_get_reg(ISR);
    if ((isr) & (1 << 2))
    {
        // 发送完成中断
        // 响应中断
        CANAR = ISR;
        CANDR = 0x04;
    }
    if ((isr) & (1 << 3))
    {
        // 信息接受中断
        // 响应中断
        CANAR = ISR;
        CANDR = 0x08;
        // 可以处理数据了
    }
}

// 发送远程帧
void can_send_remote_frame(u16 canid)
{
    // 数据长度为0 但需要设置remote标志
    u8 buffer[4] = {0};
    buffer[0] = 1 << 6;
    canid = canid << 5;
    buffer[1] = (u8)(canid >> 8);
    buffer[2] = (u8)canid;
    can_send_pack(buffer, 4);
}
/*
can数据帧
前三个字节是信息，后8个字节是数据
*/
// 发送消息 确保length小于等于8 否则这个函数会直接返回
void can_send_msg(u16 canid, u8 *dat, u8 len)
{
    short i;
    u8 buffer[16] = {0}; // 不可能比16大
    if(len>8) return;
    for (i = 0; i < len; i++)
    {
        buffer[2 + i] = dat[i];
    }
    // 标准帧第一个字节直接就是数据长度就好
    buffer[0] = len;
    // 然后是canid
    canid = canid << 5;
    buffer[1] = (u8)(canid >> 8);
    buffer[2] = (u8)canid;
    // 发送！?
    can_send_pack(buffer, len + 3);
}
// 从数据包中提取id
u16 can_extract_id(u8 *dat)
{
    u16 canid;
    canid = (dat[1] << 8 | dat[2]) >> 5;
    return canid;
}



