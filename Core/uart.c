#include <STC32G.H>
#include <intrins.h>
#include "AR_PF.h"
#define FOSC 35000000UL // 定义为无符号长整型,避免计算溢出
#define BRT (65536 - (FOSC / 115200 + 2) / 4)
// 加 2 操作是为了让 Keil 编译器
// 自动实现四舍五入运算
bit busy;
char wptr;
char rptr;
char buffer[16];

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