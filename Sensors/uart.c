#include <AI8051U.H>
#include <stdio.h>
#include <string.h>

#define FOSC 40000000L
#define BAUD 115200
#define UART_BAUD 115200
#define UART_BUF_SIZE 64  // 定义接收缓冲区大小

char wptr; // 写指针
char rptr; // 读指针
unsigned char xdata UART_RxBuffer[UART_BUF_SIZE]; // 接收数据缓冲区

void UART_Init(void)
{
    SCON = 0x50;  // 设置为模式1（8位UART，可变波特率），允许接收
    
    // 使用定时器2作为波特率发生器
    AUXR |= 0x01;  // 选择定时器2作为波特率发生器
    
    // 计算定时器2重装值：65536 - FOSC/4/BAUD
    T2L = (65536 - (FOSC/4/UART_BAUD));
    T2H = (65536 - (FOSC/4/UART_BAUD)) >> 8;
    
    AUXR |= 0x10;  // 启动定时器2
    AUXR |= 0x04;  // 定时器2为1T模式
    
    ES = 1;        // 允许串口中断
    EA = 1;        // 允许总中断

    wptr = 0x00;
	rptr = 0x00;
}

#pragma region 输出信号
void UART_SendByte(unsigned char byte)
{
    SBUF = byte;    // 将数据写入SBUF
    while(!TI);     // 等待发送完成
    TI = 0;         // 清除发送完成标志
}

void UART_SendStr(char *p)			// 发送字符串函数
{
	while (*p)									// 字符串结束标志‘\0’前循环
	{
		UART_SendByte(*(p++));						// 逐个发送字符串的字符
	}
}

void UART_SendString(unsigned char *str)
{
    unsigned char i = 0;
    
    if(str == NULL) return;
    
    while(str[i] != '\0')
    {
        SBUF = str[i];     // 直接操作SBUF寄存器
        while(!TI);        // 等待发送完成
        TI = 0;            // 清除发送完成标志
        i++;
    }
}
#pragma endregion

#pragma region 输入信号的获取
// 检查是否有接收到的数据
unsigned short int UART_Available(void)
{
    return (wptr != rptr);
}

// 读取接收到的一个字节数据
unsigned char UART_ReadByte(void)
{
    unsigned char dat;
    
    if(wptr == rptr)
        return 0;  // 缓冲区为空，返回0
    
    dat = UART_RxBuffer[rptr];
    rptr = (rptr + 1) % UART_BUF_SIZE;  // 更新读指针
    
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

void UART_Routine() interrupt 4
{
    if(RI)  // 接收中断
    {
        // 将接收到的数据存入缓冲区
        UART_RxBuffer[wptr] = SBUF;
        wptr = (wptr + 1) % UART_BUF_SIZE;  // 更新写指针
        RI = 0;  // 清除接收中断标志
    }
    
    if(TI)  // 发送中断
    {
        TI = 0;  // 清除发送中断标志
    }
}
