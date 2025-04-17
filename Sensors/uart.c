#include <AI8051U.H>
#include <stdio.h>
#include <string.h>

#define FOSC 40000000L
#define UART3_BAUD 115200  // UART3波特率
#define UART_BUF_SIZE 64   // 定义接收缓冲区大小
bit Uart3Busy;
char uart3_wptr; // 写指针
char uart3_rptr; // 读指针
unsigned char xdata UART3_RxBuffer[UART_BUF_SIZE]; // 接收数据缓冲区

void UART_Init(void)
{
    // 配置UART3
    // S3CON: 0x50 = 0b01010000
    // bit7=0: S3SM0, UART模式0 
    // bit6=1: S3ST3, 使用定时器3作为波特率发生器
    // bit5=0: S3SM2, 禁用多机通信
    // bit4=1: S3REN, 允许接收
    // bit3-0=0000: 其他位清零
    // S3CON = 0x50;
    S3CON = 0x10;		//8位数据,可变波特率
	S3CON |= 0x40;		//串口3选择定时器3为波特率发生器
    
    P_SW2 |= 0x02;  // 将UART3切换到P5.0(RXD3)和P5.1(TXD3)
    
    // 配置波特率
    // 使用定时器3作为波特率发生器
    T4T3M &= ~0x0f;        // 清除定时器3相关位
    T4T3M |= 0x0a;         // 定时器3为1T模式，启动定时器3
    
    // 计算定时器3重装值：65536 - FOSC/4/UART3_BAUD
    T3L = (65536 - (FOSC/4/UART3_BAUD));
    T3H = (65536 - (FOSC/4/UART3_BAUD)) >> 8;
    
    IE2 |= 0x08;           // 使能UART3中断
    EA = 1;                // 允许总中断

    uart3_wptr = 0x00;
    uart3_rptr = 0x00;

    P5M0 |= 0x03; P5M1 &= ~0x03; // 设置TXD3和RXD3为推挽输出模式
}

#pragma region 输出信号
void UART_SendByte(unsigned char byte)
{
    while(Uart3Busy);
    Uart3Busy = 1;
    S3BUF = byte;          // 将数据写入S3BUF
    
}

void UART_SendStr(char *p)
{
    while (*p)
    {
        UART_SendByte(*(p++));
    }
}

void UART_SendString(unsigned char *str)
{
    unsigned char i = 0;
    
    if(str == NULL) return;
    
    while(str[i] != '\0')
    {
        S3BUF = str[i];           // 直接操作S3BUF寄存器
        while(!(S3CON & 0x02));   // 等待发送完成
        S3CON &= ~0x02;           // 清除发送完成标志
        i++;
    }
}

void Uart_SendByLength(unsigned char *p,int length)
{
    int i;
    p+=length-1; // 将指针移动到最后一个字节
    for(i=0;i<length;i++) //!这里是小端储蓄模式，至于为什么这样做?我草你马stc8051！你写什么jb玩意寄存器？诗人我吃你nm的
    {
        UART_SendByte(*(p--));
    }
}

void UART_SendFloat(float value[19]) // 发送浮点数数据
{
    unsigned char *p;
    unsigned int i, j;
    
    // 发送浮点数数据（以字节方式）
    for (i = 0; i < 19; i++)
    {
        Uart_SendByLength((unsigned char *)&value[i], 4); //?nnd stc51的寄存器真是恶心
    }
    
    // 发送FireWater协议帧尾 (0x00, 0x00, 0x80, 0x7F)
    UART_SendByte(0x00);
    UART_SendByte(0x00);
    UART_SendByte(0x80);
    UART_SendByte(0x7F);
}

#pragma endregion

#pragma region 输入信号的获取
// 检查是否有接收到的数据
unsigned short int UART_Available(void)
{
    return (uart3_wptr != uart3_rptr);
}

// 读取接收到的一个字节数据
unsigned char UART_ReadByte(void)
{
    unsigned char dat;
    
    if(uart3_wptr == uart3_rptr)
        return 0;  // 缓冲区为空，返回0
    
    dat = UART3_RxBuffer[uart3_rptr];
    uart3_rptr = (uart3_rptr + 1) % UART_BUF_SIZE;  // 更新读指针
    
    return dat;
}

// 读取接收缓冲区中的多个字节
unsigned char UART_Read(unsigned char *buf, unsigned char len)
{
    unsigned char i = 0;
    
    while(UART_Available() && i < len)
    {
        buf[i++] = UART_ReadByte();
    }
    
    return i;  // 返回实际读取的字节数
}
#pragma endregion

// UART3中断服务程序
void UART3_Routine() interrupt 17
{
    if (S3TI)
    {
        S3TI = 0;
        Uart3Busy = 0;
    }
    if (S3RI)
    {
        S3RI = 0;
        UART3_RxBuffer[uart3_wptr++] = S3BUF;
        uart3_wptr &= 0x0f;
    }
}