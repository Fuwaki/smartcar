#include <STC32G.H>
#include <intrins.h>
#define FOSC 35000000UL
#define BRT (65536 - (FOSC / 115200 + 2) / 4)
bit busy;
char wptr;
char rptr;
char buffer[32];
char bufferFuaking[32];
char *s = "%d"; // 移除多余的\0

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

    // 启用串口3中断
    ES3 = 1; // 需要在STC32G.H中定义，如果没有定义，可以用INTCLKO寄存器中的相应位
    EA = 1;  // 开启总中断
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
void Uart3SendByLength(unsigned char *p,int length)
{
    int i;
    p+=length-1;
    for(i=0;i<length;i++)
    {
        Uart3Send(*(p--));
    }
}


// // 将浮点数转换为ASCII字符串，保留6位小数
// void FloatToStr(char *str, double num)
// {
//     int intPart;
//     double decPart;
//     int i, j;
//     char temp[20];
    
//     // 处理负数
//     if (num < 0)
//     {
//         *str++ = '-';
//         num = -num;
//     }
    
//    // 分离整数部分和小数部分
//     intPart = (int)num;
//     decPart = num - intPart;
    
//     // 转换整数部分
//     i = 0;
//     do {
//         temp[i++] = intPart % 10 + '0';
//         intPart /= 10;
//     } while (intPart > 0);
    
//     // 逆序复制整数部分到输出字符串
//     for (j = i - 1; j >= 0; j--)
//     {
//         *str++ = temp[j];
//     }
    
//     // 添加小数点
//     *str++ = '.';
    
//     // 转换小数部分（保留6位小数）
//     for (i = 0; i < 6; i++)
//     {
//         decPart *= 10;
//         *str++ = (int)decPart + '0';
//         decPart -= (int)decPart;
//     }
    
//     // 添加字符串结束符
//     *str = '\0';
// }

void VOFA_SendFloat(float value[3])
{
    unsigned char *p;
    unsigned int i, j;

    // 发送浮点数数据（以字节方式）
    for (i = 0; i < 3; i++)
    {
        Uart3SendByLength((unsigned char *)&value[i], 4);
    }

    // 发送FireWater协议帧尾 (0x00, 0x00, 0x80, 0x7F)
    Uart3Send(0x00);
    Uart3Send(0x00);
    Uart3Send(0x80);
    Uart3Send(0x7F);
}