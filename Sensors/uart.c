#include <AI8051U.H>
#include <stdio.h>
#include <string.h>

#define FOSC 40000000L     // 系统时钟频率
#define UART2_BAUD 115200  // UART2波特率
#define UART_BUF_SIZE 64   // 定义接收缓冲区大小

char uart2_wptr; // 写指针
char uart2_rptr; // 读指针
unsigned char xdata UART2_RxBuffer[UART_BUF_SIZE]; // 接收数据缓冲区

void UART_Init(void)
{
    // 配置UART2
    S2CON = 0x50;  // 设置为模式1（8位UART，可变波特率），允许接收
    
    P_SW2 |= 0x01;  // 将UART2切换到P1.0(RX2)和P1.1(TX2)
    
    // 配置波特率
    // 使用定时器3作为波特率发生器
    T4T3M &= ~0x0f;        // 清除定时器3相关位
    T4T3M |= 0x0a;         // 定时器3为1T模式，启动定时器3
    
    // 计算定时器3重装值：65536 - FOSC/4/UART2_BAUD
    T3L = (65536 - (FOSC/4/UART2_BAUD));
    T3H = (65536 - (FOSC/4/UART2_BAUD)) >> 8;
    
    // 配置定时器3为UART2的波特率发生器
    S2CON |= 0x04;         // 选择Timer3作为波特率发生器
    
    IE2 |= 0x01;           // 使能UART2中断
    EA = 1;                // 允许总中断

    uart2_wptr = 0x00;
    uart2_rptr = 0x00;
}

#pragma region 输出信号
void UART_SendByte(unsigned char byte)
{
    S2BUF = byte;          // 将数据写入S2BUF
    while(!(S2CON & 0x02)); // 等待发送完成
    S2CON &= ~0x02;        // 清除发送完成标志
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
        S2BUF = str[i];           // 直接操作S2BUF寄存器
        while(!(S2CON & 0x02));   // 等待发送完成
        S2CON &= ~0x02;           // 清除发送完成标志
        i++;
    }
}
#pragma endregion

#pragma region 输入信号的获取
// 检查是否有接收到的数据
unsigned short int UART_Available(void)
{
    return (uart2_wptr != uart2_rptr);
}

// 读取接收到的一个字节数据
unsigned char UART_ReadByte(void)
{
    unsigned char dat;
    
    if(uart2_wptr == uart2_rptr)
        return 0;  // 缓冲区为空，返回0
    
    dat = UART2_RxBuffer[uart2_rptr];
    uart2_rptr = (uart2_rptr + 1) % UART_BUF_SIZE;  // 更新读指针
    
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

// UART2中断服务程序
void UART2_Routine() interrupt 8
{
    if(S2CON & 0x01)  // 接收中断
    {
        // 将接收到的数据存入缓冲区
        UART2_RxBuffer[uart2_wptr] = S2BUF;
        uart2_wptr = (uart2_wptr + 1) % UART_BUF_SIZE;  // 更新写指针
        S2CON &= ~0x01;  // 清除接收中断标志
    }
    
    if(S2CON & 0x02)  // 发送中断
    {
        S2CON &= ~0x02;  // 清除发送中断标志
    }
}