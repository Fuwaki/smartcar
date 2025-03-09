#include "can.h"
#include <common.h>
typedef unsigned char u8;
typedef unsigned int u16;
u8 rx_buf[8];
u8 tx_buf[8];
//��׼֡����Ϣ���ȶ���8
void can_init()
{
    // TODO: ���ò�����

    // �л����ŵ�p00��p01
    CAN_S1 = 0;
    CAN_S0 = 0;
    CANICR = 0x02; // ����CAN�ж�
    CANEN = 1;     // ����CANģ��
}
// ʹ�ñ�׼֡�ͺ�
void can_interrupt() interrupt 28
{
    u8 isr;
    isr = can_get_reg(ISR);
    if ((isr) & (1 << 2) == (1 << 2))
    {
        // ��������ж�
        CANAR = 0x03;
        CANDR = 0x04;
    }
    if ((isr) & (1 << 3) == (1 << 3))
    {
        // ��Ϣ�����ж�
        


    }
}
// can�����мĴ�����Ҫ��ӻ�ȡ
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
// ����ң��֡
/*
01 ����ת��
00 ���ת��
10 ��õ���
*/