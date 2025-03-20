#ifndef __UART_DEBUG_H__
#define __UART_DEBUG_H__
    void VOFA_SendFloat(float value[3]);
    void Uart3Init();
    void Uart3Send(char dat);
    void Uart3SendStr(char *p);
    void Uart3SendByLength(char *p,int length);

#endif