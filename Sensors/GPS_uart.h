#ifndef __GPS_UART_H__
#define __GPS_UART_H__
    void GPS_UART_Init();
    void GPS_UART_SendByte(unsigned char byte);
    void GPS_UART_SendStr(char *p);
    void GPS_UART_SendString(unsigned char *str);
    unsigned short int GPS_UART_Available(void);
    unsigned char GPS_UART_ReadByte(void);
    unsigned char GPS_UART_Read(unsigned char *buf);
#endif
