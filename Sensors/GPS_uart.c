#include <stdio.h>
#include <AI8051U.H>
#include <string.h>

#define FOSC 40000000L
#define BAUD 115200
#define UART_BAUD 115200
#define UART_BUF_SIZE 64                          // 定义接收缓冲区大小
unsigned char wptr;                               // 写指针
unsigned char rptr;                               // 读指针
unsigned char xdata UART_RxBuffer[UART_BUF_SIZE]; // 接收数据缓冲区
void GPS_UART_Init()
{
    // 设置UART1使用RXD_2/TXD_2引脚
    P_SW1 &= ~0xC0; // 清除S1_S1和S1_S0位
    P_SW1 |= 0x40;  // 设置S1_S0=1, S1_S1=0，选择RXD_2/TXD_2
    SCON = 0x50;    // 设置为模式1（8位UART，可变波特率），允许接收
    // 使用定时器2作为波特率发生器
    AUXR |= 0x01; // 选择定时器2作为波特率发生器
    // 计算定时器2重装值：65536 - FOSC/4/BAUD
    T2L = (unsigned char)(65536 - (FOSC / 4 / UART_BAUD));      // 低8位
    T2H = (unsigned char)((65536 - (FOSC / 4 / UART_BAUD)) >> 8); // 高8位

    AUXR |= 0x10;   // 启动定时器2
    AUXR |= 0x04;   // 定时器2为1T模式

    // 设置串口中断优先级
    IP = (IP & ~0x10) | 0x10;   // 设置串口中断为高优先级
    
    ES = 1; // 允许串口中断
    EA = 1; // 允许总中断

    wptr = 0x00;
    rptr = 0x00;
    //? 验证是否正确
}

#pragma region 输出信号
void GPS_UART_SendByte(unsigned char byte)
{
    SBUF = byte; // 将数据写入SBUF
    while (!TI)
        ;   // 等待发送完成
    TI = 0; // 清除发送完成标志
}

//!请使用X系列单片机的UART发送函数
void GPS_UART_SendStr(char *p) // 发送字符串函数
{
    while (*p) // 字符串结束标志‘\0’前循环
    {
        GPS_UART_SendByte(*(p++)); // 逐个发送字符串的字符
    }
}

void GPS_UART_SendString(unsigned char *str)
{
    unsigned char i = 0;

    if (str == NULL)
        return;

    while (str[i] != '\0')
    {
        SBUF = str[i]; // 直接操作SBUF寄存器
        while (!TI)
            ;   // 等待发送完成
        TI = 0; // 清除发送完成标志
        i++;
    }
}
#pragma endregion

#pragma region 输入信号的获取
// 检查是否有接收到的数据
unsigned short int GPS_UART_Available(void)
{
    return (wptr != rptr);
}

// 读取接收到的一个字节数据
unsigned char GPS_UART_ReadByte(void)
{
    unsigned char dat;

    if (wptr == rptr)
        return 0; // 缓冲区为空，返回0

    dat = UART_RxBuffer[rptr];
    rptr = (rptr + 1) % UART_BUF_SIZE; // 更新读指针

    return dat;
}

// 读取接收缓冲区中的多个字节
unsigned char GPS_UART_Read(unsigned char *buf, unsigned char len)
{
    unsigned char i = 0;

    while (GPS_UART_Available() && i < len)
    {
        buf[i++] = GPS_UART_ReadByte();
    }

    return i; // 返回实际读取的字节数
}

#pragma endregion

void GPS_UART_Routine() interrupt 4
{
    if (RI) // 接收中断
    {
        // 将接收到的数据存入缓冲区
        UART_RxBuffer[wptr] = SBUF;
        wptr = (wptr + 1) % UART_BUF_SIZE; // 更新写指针
        RI = 0;                            // 清除接收中断标志
        
        // // 检查缓冲区是否快要溢出
        // if(((wptr + 1) % UART_BUF_SIZE) == rptr) 
        //{
        //     // 缓冲区即将溢出，可以在这里添加处理逻辑
        //     // 例如: 丢弃最旧的数据
        //     rptr = (rptr + 1) % UART_BUF_SIZE;
        // }
    }

    if (TI) // 发送中断
    {
        TI = 0; // 清除发送中断标志
    }
}

//*这里没有vofa的发送函数，因为不需要使用vofa的发送函数