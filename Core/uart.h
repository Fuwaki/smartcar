#ifndef __UART_H__
#define __UART_H__

    // 串口初始化
    void Uart3Init(); //使用的是串口3，且使用定时器2作为波特率发生器

    // 发送单个字节
    void Uart3Send(char dat);

    // 发送字符串
    void UART_SendStr(char *p);
    void Uart3SendStr(char *p);
    void Uart3SendByLength(unsigned char *p,int length);
    
#endif