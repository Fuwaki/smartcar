#include <STC32G.H>
#include <intrins.h>
#include "AR_PF.h"
#include "uart.h"
#include <string.h>
#define FOSC 35000000UL // 定义为无符号长整型,避免计算溢出
#define BRT (65536 - (FOSC / 115200 + 2) / 4)
// 加 2 操作是为了让 Keil 编译器
// 自动实现四舍五入运算
bit busy3;
unsigned char wptr3;
unsigned char rptr3;
char buffer3[16];

bit busy;
char wptr;
char rptr;
char buffer[128];

SENSOR_DATA sensor_data; // 临时传感器数据结构体
MOTOR_CONTROL_FRAME motor_control_frame; // 电机控制帧结构体

unsigned char receive_buffer[128]; // 接收缓冲区
unsigned char receive_state = 0;   // 接收状态

void Uart3Isr() interrupt 17
{
    if (S3TI)
    {
        S3TI = 0;
        busy3 = 0;
    }
    if (S3RI)
    {
        S3RI = 0;
        buffer3[wptr3++] = S3BUF;
        wptr3 &= 0x0f;
    }
}

void Uart_Isr(void) interrupt 4
{
    if (RI)	//检测串口1接收中断
    {
        RI = 0;	//清除串口1接收中断请求位
        buffer[wptr++] = SBUF;	//将接收到的数据存入缓冲区
        wptr %= 128;	//循环缓冲区指针
    }
    if (TI)	//检测串口1发送中断
    {
        TI = 0;	//清除串口1发送中断请求位
        busy = 0;	//清除发送忙标志位
    }
}

void Uart3Init() //使用的是串口3，且使用定时器3作为波特率发生器
{
    S3CON = 0x50;
    T3L = BRT;
    T3H = BRT >> 8;
    T3x12 = 1;
    T3R = 1;
    wptr3 = 0x00;
    rptr3 = 0x00;
    busy3 = 0;

    EA = 1; // 使能总中断
    ES3 = 1; // 使能串口3中断
}

void Uart1Init()
{
    P_SW1 = (P_SW1 & ~0xc0) | 0x80;		//UART1/USART1: 红的 RxD(P1.6), TxD(P1.7)
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x40;		//定时器时钟1T模式
	AUXR &= 0xFE;		//串口1选择定时器1为波特率发生器
	TMOD &= 0x0F;		//设置定时器模式
	TL1 = 0xB4;			//设置定时初始值
	TH1 = 0xFF;			//设置定时初始值
	ET1 = 0;			//禁止定时器中断
	TR1 = 1;			//定时器1开始计时
	ES = 1;				//使能串口1中断
    EA = 1;				//允许总中断
}

void Uart3Send(char dat)
{
    while (busy3)
        ;
    busy3 = 1;
    S3BUF = dat;
}

void UartSend(char dat)
{
    while (busy)
        ;
    busy = 1;
    SBUF = dat; // 将数据写入SBUF寄存器
}

void Uart3SendStr(char *p)
{
    while (*p)
    {
        Uart3Send(*(p++));
    }
}

void UartSendStr(char *p)
{
    while (*p)
    {
        UartSend(*(p++));
    }
}
// 添加函数用于检查和处理接收缓冲区
void Uart3CheckAndReceive(void)
{
    if (rptr3 != wptr3)
    {
        Uart3Send(buffer3[rptr3++]);
        rptr3 &= 0x0f;
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

void UartSendByLength(unsigned char *p,int length)
{
    int i;
    p+=length-1;
    for(i=0;i<length;i++)
    {
        UartSend(*(p--));
    }
}

// 接收 vofaFireWater 协议数据，并存入 SENSOR_DATA 结构体
void UartReceiveSensorData(void)
{
    unsigned char data_byte;
    unsigned char check_frame[4] = {0x00, 0x00, 0x80, 0x7f}; // 帧头
    // 将数据存入SENSOR_DATA结构体
    unsigned char *p = (unsigned char *)&sensor_data;
    unsigned int i;
    
    while (rptr != wptr)
    {
        data_byte = buffer[rptr++];
        rptr %= 128; // 循环缓冲区指针
        
        if (receive_state < 4) // 检查帧头
        {
            if (data_byte == check_frame[receive_state])
            {
                receive_state++;
                if (receive_state == 4) // 帧头接收完成
                {
                    // 重置接收缓冲区索引，准备接收数据
                    receive_state = 4;
                }
            }
            else
            {
                // 帧头不匹配，重置状态
                receive_state = 0;
                // 如果当前字节可能是帧头的第一个字节，需要重新检查
                if (data_byte == check_frame[0])
                {
                    receive_state = 1;
                }
            }
        }
        else if (receive_state >= 4 && receive_state < 80) // 接收18个浮点数 (4帧头 + 19*4=76字节)
        {
            receive_buffer[receive_state - 4] = data_byte;
            receive_state++;
            
            if (receive_state == 80) // 数据接收完成
            {
                // 由于发送时使用的是小端模式反向发送，接收时需要反向处理
                for (i = 0; i < 18; i++)
                {
                    p[i*4] = receive_buffer[i*4+3];
                    p[i*4+1] = receive_buffer[i*4+2];
                    p[i*4+2] = receive_buffer[i*4+1];
                    p[i*4+3] = receive_buffer[i*4];
                }
                
                receive_state = 0; // 重置状态，准备接收下一帧
            }
        }
        else
        {
            receive_state = 0; // 状态错误，重置
            // 检查是否是新帧头的开始
            if (data_byte == check_frame[0])
            {
                receive_state = 1;
            }
        }
    }
}

void UART_SendFloat(float value[18]) // 发送浮点数数据
{
    unsigned char *p;
    unsigned int i, j;
    
    // 发送浮点数数据（以字节方式）
    for (i = 0; i < 18; i++)
    {
        UartSendByLength((unsigned char *)&value[i], 4); //?nnd stc51的寄存器真是恶心
    }
    UartSend(0x00);
    UartSend(0x00);
    UartSend(0x80);
    UartSend(0x7F);
}

void UART3_SendCommandToMotor()
{
    unsigned char *p = (unsigned char *)&motor_control_frame;
    
    Uart3SendByLength(p, sizeof(MOTOR_CONTROL_FRAME));
    Uart3Send(0x00);
    Uart3Send(0x00);
    Uart3Send(0x80);
    Uart3Send(0x7F);
}