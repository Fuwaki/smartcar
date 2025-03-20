#ifndef __UART_DEBUG_H__
#define __UART_DEBUG_H__
    void VOFA_SendFloat(float value);
    void VOFA_SendFrameEnd(void);
    void VOFA_SendFloats(float *values, unsigned char count);
    void VOFA_SendCSV(float *values, unsigned char count);
    void Uart3Init();
    void Uart3Send(char dat);
    void Uart3SendStr(char *p);
#endif