#include "can.h"
#include <STC32G.H>

static can_receive_callback_t can_receive_callback = 0;
int count = 0;
void can_set_receive_callback(can_receive_callback_t callback)
{
    can_receive_callback = callback;
}
// can的所有寄存器需要间接获取
void can_set_reg(unsigned char addr, unsigned char dat)
{
    CANAR = addr;
    CANDR = dat;
}
unsigned char can_get_reg(unsigned char addr)
{
    unsigned char dat;
    CANAR = addr;
    dat = CANDR;
    return dat;
}
enum CAN_FRAME_TYPE
{
    CAN_DATA_FRAME = 0,
    CAN_REMOTE_FRAME = 1,
};
// mask为1代表着不care
void can_set_filter(unsigned int cr, unsigned int mask)
{
    can_set_reg(ACR0, (unsigned char)(cr >> 3));
    can_set_reg(ACR1, (unsigned char)(cr << 5));
    can_set_reg(AMR0, (unsigned char)(mask >> 3));
    can_set_reg(AMR1, (unsigned char)(mask << 5) | 0x1f);
    can_set_reg(ACR2, 0);
    can_set_reg(ACR3, 0);
    can_set_reg(AMR2, 0xff);
    can_set_reg(AMR3, 0xff);
}
void can_read_rx_fifo(unsigned char *array)
{
    array[0] = can_get_reg(RX_BUF0);
    array[1] = can_get_reg(RX_BUF1);
    array[2] = can_get_reg(RX_BUF2);
    array[3] = can_get_reg(RX_BUF3);
}
// stc 生成的设置波特率
void can_set_baudrate(void) // 800Kbps@35MHz
{
    can_set_reg(MR, 0x04);   // 使能Reset模式
    can_set_reg(BTR0, 0x00); // SJW(0), BRP(6)
    can_set_reg(BTR1, 0xcf); // SAM(0), TSG2(2), TSG1(15)
    can_set_reg(MR, 0x00);   // 退出Reset模式
}
// 发送封装好的数据包
void can_send_pack(unsigned char *dat, unsigned char len)
{
    unsigned short i;
    for (i = 0; i < len; i += 4)
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
    count = 0;
    CANEN = 1;
    CANSEL = 0; // 选择第一组can
    // 选择引脚
    CAN_S1 = 0;
    CAN_S0 = 0;
    can_set_reg(MR, 0x04);
    // TODO: 设置波特率
    can_set_baudrate();
    // 切换引脚到p00和p01
    can_set_reg(ACR0, 0x00); // 使能接收
    can_set_reg(ACR1, 0x00); // 使能接收
    can_set_reg(ACR2, 0x00); // 使能接收
    can_set_reg(ACR3, 0x00); // 使能接收
    can_set_reg(AMR0, 0xFF); // 使能接收
    can_set_reg(AMR1, 0xFF); // 使能接收
    can_set_reg(AMR2, 0xFF); // 使能接收
    can_set_reg(AMR3, 0xFF); // 使能接收

    can_set_reg(IMR, 0xFF);
    can_set_reg(ISR, 0xff); // 清除中断标志
    can_set_reg(MR, 0x01);  // 单滤波模式

    //TODO:看看设置高中断优先级会不会好点
    PCANH=1;
    PCANL=1;

    CANICR = 0x02; // 启用CAN中断
}
// 返回的size就是数据的size 不包含头 请保证dat足够大
enum CAN_FRAME_TYPE can_recv_msg(unsigned char *dat, unsigned char *size)
{
    unsigned char len = 0;
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
    unsigned char isr;
    unsigned char arTemp;
    int i = 0;
    unsigned char buffer[4];
    
    arTemp = CANAR; // CANAR现场保存，避免主循环里写完 CANAR
                    // 后产生中断，在中断里修改了 CANAR 内容

    isr = can_get_reg(ISR);
    // for(i=0;i<isr;i++){
    // }
    P40=~P40;

    if ((isr & 0x04) == 0x04) // TI
    {
        CANAR = ISR;
        CANDR |= 0x04; // CLR FLAG
    }

    if ((isr & 0x08) == 0x08) // RI
    {
        CANAR = ISR;
        CANDR |= 0x08; // CLR FLAG
        can_read_rx_fifo(buffer);
    }

    if ((isr & 0x40) == 0x40) // ALI
    {
        CANAR = ISR;
        CANDR |= 0x40; // CLR FLAG
    }

    if ((isr & 0x20) == 0x20) // EWI
    {
        CANAR = ISR;
        CANDR |= 0x20; // CLR FLAG
    }

    if ((isr & 0x10) == 0x10) // EPI
    {
        CANAR = ISR;
        CANDR |= 0x10; // CLR FLAG
    }

    if ((isr & 0x02) == 0x02) // BEI
    {
        CANAR = ISR;
        CANDR |= 0x02; // CLR FLAG
    }

    if ((isr & 0x01) == 0x01) // DOI
    {
        CANAR = ISR;
        CANDR |= 0x01; // CLR FLAG
    }

    CANAR = arTemp; // CANAR现场恢复
}
void can_debug()
{
    unsigned char sr = can_get_reg(SR);
    unsigned int i;
    for (i = 0; i < sr; i++)
    {
        P40 = ~P40;
    }
}
// 发送远程帧
void can_send_remote_frame(unsigned int canid)
{
    // 数据长度为0 但需要设置remote标志
    unsigned char buffer[4] = {0};
    buffer[0] = 1 << 6;
    canid = canid << 5;
    buffer[1] = (unsigned char)(canid >> 8);
    buffer[2] = (unsigned char)canid;
    can_send_pack(buffer, 4);
}
/*
can数据帧
前三个字节是信息，后8个字节是数据
*/
// 发送消息 确保length小于等于8 否则这个函数会直接返回
void can_send_msg(unsigned int canid, unsigned char *dat, unsigned char len)
{
    short i;
    unsigned char buffer[16] = {0}; // 不可能比16大
    // 检查SR
    unsigned char sr = can_get_reg(SR);
    // unsigned char ecc=can_get_reg(ALC);
    count++;

    // 32 35 3

    if (sr & 0x01) // 判断是否有 BS:BUS-OFF状态
    {
        CANAR = MR;
        CANDR &= ~0x04; // 清除 Reset Mode, 从BUS-OFF状态退出
    }

    if (len > 8)
        return;
    for (i = 0; i < len; i++)
    {
        buffer[3 + i] = dat[i];
    }
    // 标准帧第一个字节直接就是数据长度就好
    buffer[0] = len;
    // 然后是canid
    canid = canid << 5;
    buffer[1] = (unsigned char)(canid >> 8);
    buffer[2] = (unsigned char)canid;
    // 发送！?
    can_send_pack(buffer, len + 3);
}
// 从数据包中提取id
unsigned int can_extract_id(unsigned char *dat)
{
    unsigned int canid;
    canid = ((dat[1] << 8) + dat[2]) >> 5;
    return canid;
}
