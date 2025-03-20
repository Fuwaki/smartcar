#include <STC32G.H>
#include <intrins.h>
#include "AR_PF.h"
#define FOSC 35000000UL
#define BRT (65536 - (FOSC / 115200 + 2) / 4)
bit busy;
char wptr;
char rptr;
char buffer[16];

// VOFA FireWater帧结束符
const unsigned char VOFA_FRAME_END[4] = {0x00, 0x00, 0x80, 0x7f};

// 函数声明
void Uart3Send(char dat);
void Uart3SendStr(char *p);

// 发送浮点数据到VOFA（FireWater格式）
void VOFA_SendFloat(float value)
{
    unsigned char i;
    union {
        float f;
        unsigned char bytes[4];
    } converter;
    
    converter.f = value;
    
    for (i = 0; i < 4; i++)
    {
        Uart3Send(converter.bytes[i]);
    }
}

// 发送VOFA帧结束符（FireWater格式需要）
void VOFA_SendFrameEnd(void)
{
    unsigned char i;
    for (i = 0; i < 4; i++)
    {
        Uart3Send(VOFA_FRAME_END[i]);
    }
}

// 发送多个浮点数据到VOFA（FireWater格式）
void VOFA_SendFloats(float *values, unsigned char count)
{
    unsigned char i;
    for (i = 0; i < count; i++)
    {
        VOFA_SendFloat(values[i]);
    }
    VOFA_SendFrameEnd();
}

// 发送CSV格式数据到VOFA
void VOFA_SendCSV(float *values, unsigned char count)
{
    char buffer[64]; // 确保缓冲区足够大
    unsigned char pos = 0;
    unsigned char i;
    for (i = 0; i < count - 1; i++)
    {
        pos += UZ_sprintf(buffer + pos, "%f,", values[i]);
    }
    
    // 最后一个值后跟换行符
    if (count > 0)
    {
        pos += UZ_sprintf(buffer + pos, "%f\r\n", values[count - 1]);
    }
    
    Uart3SendStr(buffer);
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

void Uart3Init(void)
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
        Uart3Send(*p);
        p++;
    }
}

// void main()
// {
//     char c[10];
//     float testData[3]; // 测试数据数组
//     unsigned char dataCounter = 0;
    
//     EAXFR = 1;    // 使能访问 XFR,没有冲突不用关闭
//     CKCON = 0x00; // 设置外部数据总线速度为最快
//     WTST = 0x00;  // 设置程序代码等待参数，
//     // 赋值为 0 可将 CPU 执行程序的速度设置为最快
//     P0M0 = 0x00;
//     P0M1 = 0x00;
//     P1M0 = 0x00;
//     P1M1 = 0x00;
//     P2M0 = 0x00;
//     P2M1 = 0x00;
//     P3M0 = 0x00;
//     P3M1 = 0x00;
//     P4M0 = 0x00;
//     P4M1 = 0x00;
//     P5M0 = 0x00;
//     P5M1 = 0x00;
    
//     Uart3Init();
//     ES3 = 1;
//     EA = 1;
    
//     c[0] = 'V';
//     c[1] = 'O';
//     c[2] = 'F';
//     c[3] = 'A';
//     c[4] = ':';
//     c[5] = ' ';
//     c[6] = 0;

//     while (1)
//     {
//         if (rptr != wptr)
//         {
//             Uart3Send(buffer[rptr++]);
//             rptr &= 0x0f;
//         }
        
//         // 生成测试数据 - 模拟正弦波、余弦波和锯齿波
//         testData[0] = (float)dataCounter / 10.0f;           // 锯齿波
//         testData[1] = 2.5f * (float)__sinf(dataCounter);    // 正弦波
//         testData[2] = 1.5f * (float)__cosf(dataCounter);    // 余弦波
//         dataCounter++;
        
//         if (dataCounter >= 100) 
//             dataCounter = 0;
        
//         // 使用FireWater格式发送数据
//         VOFA_SendFloats(testData, 3);
        
//         // 或者使用CSV格式发送数据
//         // VOFA_SendCSV(testData, 3);
        
//         // 打印信息
//         Uart3SendStr(c);
        
//     }
// }