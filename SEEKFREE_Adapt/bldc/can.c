#include "can.h"
#include <common.h>
typedef unsigned char u8;
typedef unsigned int u16;
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
enum CAN_FRAME_TYPE
{
    CAN_DATA_FRAME = 0,
    CAN_REMOTE_FRAME = 1,
};
void can_read_rx_fifo(u8 *array)
{
    array[0] = can_get_reg(RX_BUF0);
    array[1] = can_get_reg(RX_BUF1);
    array[2] = can_get_reg(RX_BUF2);
    array[3] = can_get_reg(RX_BUF3);
}
// stc ���ɵ����ò�����
void can_set_baudrate(void) // 125Kbps@30MHz
{
    can_set_reg(MR, 0x04);   // ʹ��Resetģʽ
    can_set_reg(BTR0, 0x04); // SJW(0), BRP(4)
    can_set_reg(BTR1, 0x6f); // SAM(0), TSG2(6), TSG1(15)
    can_set_reg(MR, 0x00);   // �˳�Resetģʽ
}
// ���ͷ�װ�õ����ݰ� ����һ��Ҫ��4�ı���
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
    can_set_reg(CMR, 0x04); // ����������
}
// ��׼֡����Ϣ���ȶ���8
void can_init()
{
    // TODO: ���ò�����
    can_set_baudrate();

    // �л����ŵ�p00��p01
    CAN_S1 = 0;
    CAN_S0 = 0;
    CANICR = 0x02; // ����CAN�ж�
    CANEN = 1;     // ����CANģ��
}
// ���ص�size�������ݵ�size ������ͷ �뱣֤dat�㹻��
enum CAN_FRAME_TYPE can_recv_msg(u8 *dat, u8 *size)
{
    u8 len = 0;
    short i;
    can_read_rx_fifo(dat);
    len = (dat[0] & 0xf) - 1; // ��ֻҪ�Ͷ˵���λ �����Ѿ�����һ���ֽ���
    *size = len + 1;
    for (i = 0; i < len + 4; i += 4)
    {
        can_read_rx_fifo(dat + i);
    }
    return dat[0] & (1 << 6) ? CAN_REMOTE_FRAME : CAN_DATA_FRAME;
}
// ʹ�ñ�׼֡�ͺ�
void can_interrupt() interrupt 28
{
    u8 isr;
    isr = can_get_reg(ISR);
    if ((isr) & (1 << 2) == (1 << 2))
    {
        // ��������ж�
        // ��Ӧ�ж�
        CANAR = ISR;
        CANDR = 0x04;
    }
    if ((isr) & (1 << 3) == (1 << 3))
    {
        // ��Ϣ�����ж�
        // ��Ӧ�ж�
        CANAR = ISR;
        CANDR = 0x08;
        // ���Դ���������
    }
}

// ����Զ��֡
void can_send_remote_frame(u16 canid)
{
    // ���ݳ���Ϊ0 ����Ҫ����remote��־
    u8 buffer[4] = {0};
    buffer[0] = 1 << 6;
    canid = canid << 5;
    buffer[1] = (u8)(canid >> 8);
    buffer[2] = (u8)canid;
    can_send_pack(buffer, 4);
}
/*
can����֡
ǰ�����ֽ�����Ϣ����8���ֽ�������
*/
// ������Ϣ ȷ��lengthС�ڵ���8
void can_send_msg(u16 canid, u8 *dat, u8 len)
{
    short i;
    u8 buffer[16] = {0}; // �����ܱ�16��
    for (i = 0; i < len; i++)
    {
        buffer[2 + i] = dat[i];
    }
    // ��׼֡��һ���ֽ�ֱ�Ӿ������ݳ��Ⱦͺ�
    buffer[0] = len;
    // Ȼ����canid
    canid = canid << 5;
    buffer[1] = (u8)(canid >> 8);
    buffer[2] = (u8)canid;
    // ���ͣ�?
    can_send_pack(buffer, len + 3);
}
// �����ݰ�����ȡid
u16 can_extract_id(u8 *dat)
{
    u16 canid;
    canid = (dat[1] << 8 | dat[2]) >> 5;
    return canid;
}

// ����ң��֡
/*
01 ����ת��
00 ���ת��
10 ��õ���
*/