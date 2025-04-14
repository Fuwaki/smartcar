#include <STC32G.H>
#include "i2c.h"


// I2C总线初始化
void I2C_Init(void)
{
    P_SW2 |= 0x80;                     // 使能访问XSFR（扩展特殊功能寄存器）
    P_SW2 = (P_SW2 & ~0x30) | 0x10;		//I2C: I2CSCL(P2.5), I2CSDA(P2.4)
    // 配置I2C引脚
    P2M0 &= ~(1<<5);                   // P2.0 - SCL
    P2M1 |= (1<<5);                    // 设置为开漏模式
    P2M0 &= ~(1<<4);                   // P2.1 - SDA
    P2M1 |= (1<<4);                    // 设置为开漏模式
    
    // I2C控制器配置
    I2CCFG = 0xC0;                     // 使能I2C主机模式
    I2CMSST = 0x00;                    // 清除标志位


    I2CCFG |= 0x2C;                    // I2C时钟 = Fosc/2/(I2CCFG & 0x3F) = 400KHz左右
    
    P_SW2 &= ~0x80;                    // 禁止访问XSFR
}

void I2C_Start(void)
{
    P_SW2 |= 0x80;                     // 使能访问XSFR
    I2CMSCR = 0x01;                    // 发送START命令
    while (!(I2CMSST & 0x40));         // 等待完成
    I2CMSST &= ~0x40;                  // 清除完成标志
    P_SW2 &= ~0x80;                    // 禁止访问XSFR
}

void I2C_Stop(void)
{
    P_SW2 |= 0x80;                     // 使能访问XSFR
    I2CMSCR = 0x02;                    // 发送STOP命令
    while (!(I2CMSST & 0x40));         // 等待完成
    I2CMSST &= ~0x40;                  // 清除完成标志
    P_SW2 &= ~0x80;                    // 禁止访问XSFR
}

unsigned char I2C_SendByte(unsigned char dat)
{
    unsigned char ack;
    
    P_SW2 |= 0x80;                     // 使能访问XSFR
    I2CTXD = dat;                      // 写数据
    I2CMSCR = 0x03;                    // 发送SEND命令
    while (!(I2CMSST & 0x40));         // 等待完成
    I2CMSST &= ~0x40;                  // 清除完成标志
    
    ack = (I2CMSST & 0x02) ? 0 : 1;    // 获取ACK信号
    P_SW2 &= ~0x80;                    // 禁止访问XSFR
    
    return ack;                        // 返回应答位，0表示无应答，1表示有应答
}

unsigned char I2C_ReceiveByte(unsigned char ack)
{
    unsigned char dat;
    
    P_SW2 |= 0x80;                     // 使能访问XSFR
    
    if (ack)
        I2CMSCR = 0x04;                // 发送RECV命令，接收后发送ACK
    else
        I2CMSCR = 0x05;                // 发送RECV命令，接收后发送NACK
    
    while (!(I2CMSST & 0x40));         // 等待完成
    I2CMSST &= ~0x40;                  // 清除完成标志
    
    dat = I2CRXD;                      // 读取接收到的数据
    P_SW2 &= ~0x80;                    // 禁止访问XSFR
    
    return dat;
}

unsigned char I2C_WriteBytes(unsigned char addr, unsigned char* pData, int len)// I2C主机发送数据（地址+多个数据）
{
    int i;
    
    I2C_Start();                       // 发送起始信号
    if (!I2C_SendByte(addr << 1))      // 发送设备地址+写操作
    {
        I2C_Stop();
        return 0;                      // 无应答，返回错误
    }
    
    for (i = 0; i < len; i++)
    {
        if (!I2C_SendByte(pData[i]))   // 发送数据
        {
            I2C_Stop();
            return 0;                  // 无应答，返回错误
        }
    }
    
    I2C_Stop();                        // 发送结束信号
    return 1;                          // 操作成功
}

// I2C写命令或数据到OLED
unsigned char I2C_WriteToOLED(unsigned char addr, unsigned char* pData, int len)
{
    return I2C_WriteBytes(addr >> 1, pData, len);
}

// I2C主机读取数据
unsigned char I2C_ReadBytes(unsigned char addr, unsigned char reg, unsigned char* pData, int len)
{
    int i;
    
    I2C_Start();                       // 发送起始信号
    if (!I2C_SendByte(addr << 1))      // 发送设备地址+写操作
    {
        I2C_Stop();
        return 0;                      // 无应答，返回错误
    }
    
    if (!I2C_SendByte(reg))            // 发送寄存器地址
    {
        I2C_Stop();
        return 0;                      // 无应答，返回错误
    }
    
    I2C_Start();                       // 重新发送起始信号
    if (!I2C_SendByte((addr << 1) | 0x01))  // 发送设备地址+读操作
    {
        I2C_Stop();
        return 0;                      // 无应答，返回错误
    }
    
    for (i = 0; i < len - 1; i++)
    {
        pData[i] = I2C_ReceiveByte(1); // 读取数据并发送ACK
    }
    pData[len - 1] = I2C_ReceiveByte(0); // 最后一个字节读取后发送NACK
    
    I2C_Stop();                        // 发送结束信号
    return 1;                          // 操作成功
}

// I2C主机发送一个字节的命令或数据
void I2C_SendByteToAddr(unsigned char addr, unsigned char reg, unsigned char dat)
{
    unsigned char buffer[2];
    buffer[0] = reg;
    buffer[1] = dat;
    I2C_WriteBytes(addr >> 1, buffer, 2);
}