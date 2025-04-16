#ifndef __I2C_H__
#define __I2C_H__
#include <STC32G.H>

// I2C初始化
void I2C_Init(void);

// I2C起始信号
void I2C_Start(void);

// I2C停止信号
void I2C_Stop(void);

// I2C发送一个字节
unsigned char I2C_SendByte(unsigned char dat);

// I2C接收一个字节
unsigned char I2C_ReceiveByte(unsigned char ack);

// I2C写多个字节
unsigned char I2C_WriteBytes(unsigned char addr, unsigned char* pData, int len);

// I2C专门用于OLED的写函数
unsigned char I2C_WriteToOLED(unsigned char addr, unsigned char* pData, int len);

// I2C读多个字节
unsigned char I2C_ReadBytes(unsigned char addr, unsigned char reg, unsigned char* pData, int len);

// I2C发送一个字节到指定地址
void I2C_SendByteToAddr(unsigned char addr, unsigned char reg, unsigned char dat);

#endif /* __I2C_H__ */