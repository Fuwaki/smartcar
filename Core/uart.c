#include <STC32G.H>
#include <intrins.h>
#include "AR_PF.h"
#include "Observer.h"
#define FOSC 35000000UL // 定义为无符号长整型,避免计算溢出
#define BRT (65536 - (FOSC / 115200 + 2) / 4)
// 加 2 操作是为了让 Keil 编译器
// 自动实现四舍五入运算
bit busy;
char wptr;
char rptr;
char buffer[16];

unsigned char receive_buffer[100]; // 接收缓冲区
unsigned char receive_state = 0;   // 接收状态
unsigned int receive_count = 0;     // 接收计数器

void Uart3Isr() interrupt 17
{
    if (S3TI)
    {
        S3TI = 0;
        busy = 0;
    }
    if (S3RI)
    {
        S3RI = 0;
        buffer[wptr++] = S3BUF;
        wptr &= 0x0f;
    }
}

void Uart3Init() //使用的是串口3，且使用定时器2作为波特率发生器
{
    S3CON = 0x10;
    T2L = BRT;
    T2H = BRT >> 8;
    T2x12 = 1;
    T2R = 1;
    wptr = 0x00;
    rptr = 0x00;
    busy = 0;
}

void Uart3Send(char dat)
{
    while (busy)
        ;
    busy = 1;
    S3BUF = dat;
}

void Uart3SendStr(char *p)
{
    while (*p)
    {
        Uart3Send(*(p++));
    }
}

// 添加函数用于检查和处理接收缓冲区
void Uart3CheckAndReceive(void)
{
    if (rptr != wptr)
    {
        Uart3Send(buffer[rptr++]);
        rptr &= 0x0f;
    }
}

void Uart3SendByLength(unsigned char *p,int length)
{
    int i;
    p+=length-1;
    for(i=0;i<length;i++)
    {
        Uart3Send(*(p--));
    }
}

// 接收 vofaFireWater 协议数据，并存入 SENSOR_DATA 结构体
void Uart3ReceiveSensorData(void)
{
    unsigned char data_byte;
    unsigned char check_frame[4] = {0x00, 0x00, 0x80, 0x7F}; // FireWater协议帧尾
    
    while (rptr != wptr)
    {
        data_byte = buffer[rptr++];
        rptr &= 0x0f;
        
        if (receive_state < 72) // 接收18个浮点数 (18 * 4 = 72字节)
        {
            receive_buffer[receive_state++] = data_byte;
        }
        else if (receive_state < 76) // 检查帧尾
        {
            if (data_byte == check_frame[receive_state - 72])
            {
                receive_state++;
                if (receive_state == 76) // 帧尾接收完成
                {
                    // 将数据存入SENSOR_DATA结构体
                    unsigned char *p = (unsigned char *)&sensor_data;
                    unsigned int i;
                    
                    // 由于发送时使用的是小端模式反向发送，接收时需要反向处理
                    for (i = 0; i < 18; i++)
                    {
                        // 对于每个浮点数（4字节）进行反向处理
                        p[i*4] = receive_buffer[i*4+3];
                        p[i*4+1] = receive_buffer[i*4+2];
                        p[i*4+2] = receive_buffer[i*4+1];
                        p[i*4+3] = receive_buffer[i*4];
                    }
                    
                    receive_state = 0; // 重置状态
                }
            }
            else
            {
                // 帧尾不匹配，重置状态
                receive_state = 0;
            }
        }
        else
        {
            receive_state = 0; // 状态错误，重置
        }
    }
}