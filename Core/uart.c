#include <STC32G.H>
#include <intrins.h>
#include "AR_PF.h"
#define FOSC 11059200UL // 定义为无符号长整型,避免计算溢出
#define BRT (65536 - (FOSC / 115200 + 2) / 4)
// 加 2 操作是为了让 Keil 编译器
// 自动实现四舍五入运算
bit busy;
char wptr;
char rptr;
char buffer[16];



// char c[10]="Ahbjh";
// char *p=&c;

void Delay1000ms(void)	//@11.0592MHz
{
	unsigned long edata i;

	_nop_();
	_nop_();
	i = 2764798UL;
	while (i) i--;
}


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
void Uart3Init()
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
void main()
{
    char c[10];
    char d[3];
    char q='a';
    c[0]='A';
    c[1]=0;
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
    ES3 = 1;
    EA = 1;
    d[0]='%';
    d[1]='d';
    d[2]=0;
    UZ_sprintf(c,d,(int)&q);

    while (1)
    {
        if (rptr != wptr)
        {
            Uart3Send(buffer[rptr++]);
            rptr &= 0x0f;
        }
        // c='A';

        Uart3SendStr(c);
        // c[0]++;
        Delay1000ms();
    }
}