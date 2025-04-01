#ifndef __SPI_MULTIDEVICE_H__
#define __SPI_MULTIDEVICE_H__

#include <AI8051U.H>
#include <stdio.h>
#include <string.h>

// 最大支持的SPI从设备数量
#define MAX_SPI_SLAVES 8

// SPI模式定义
#define SPI_MODE0 0x00 // CPOL=0, CPHA=0 - 空闲时时钟为低电平，数据在时钟上升沿采样
#define SPI_MODE1 0x04 // CPOL=0, CPHA=1 - 空闲时时钟为低电平，数据在时钟下降沿采样
#define SPI_MODE2 0x08 // CPOL=1, CPHA=0 - 空闲时时钟为高电平，数据在时钟下降沿采样
#define SPI_MODE3 0x0C // CPOL=1, CPHA=1 - 空闲时时钟为高电平，数据在时钟上升沿采样

// 从设备定义结构体
typedef struct
{
    unsigned char cs_port;   // CS引脚所在端口 (0:P0, 1:P1, 2:P2, 3:P3, 4:P4, 5:P5, 6:P6, 7:P7)
    unsigned char cs_pin;    // CS引脚编号 (0-7)
    unsigned char mode;      // SPI模式 (0-3)
    unsigned char clock_div; // 时钟分频 (2/4/8/16/32/64/128)
} spi_slave_config_t;

// 初始化SPI主机
void SPI_Init(void);

// 注册从设备
// 返回从设备ID（0到MAX_SPI_SLAVES-1），失败返回0xFF
unsigned char SPI_RegisterSlave(spi_slave_config_t *slave_config);

// 选择从设备
void SPI_SelectSlave(unsigned char slave_id);

// 释放从设备
void SPI_ReleaseSlave(unsigned char slave_id);

// 发送并接收一个字节
unsigned char SPI_TransferByte(unsigned char data_out);

// 传输多个字节
void SPI_TransferBuffer(unsigned char *data_out, unsigned char *data_in, unsigned int len);

// 方便的寄存器读写函数
unsigned char SPI_ReadRegister(unsigned char slave_id, unsigned char reg_addr);
void SPI_WriteRegister(unsigned char slave_id, unsigned char reg_addr, unsigned char value);

// 读取多个寄存器
void SPI_ReadMultiRegisters(unsigned char slave_id, unsigned char start_addr,
                            unsigned char *buffer, unsigned int count);

// 写入多个寄存器
void SPI_WriteMultiRegisters(unsigned char slave_id, unsigned char start_addr,
                             unsigned char *buffer, unsigned int count);

// SPI从模式初始化函数
void SPI_InitSlave(void);
void SPI_DisableSlave(void);

// SPI从模式回调函数设置
void SPI_SetSlaveRxCallback(void (*callback)(unsigned char));
void SPI_SetSlaveTxCallback(unsigned char (*callback)(void));
void SPI_SlavePrepareTxData(unsigned char dataSend);

// 新增: 准备发送浮点数数组的函数
void SPI_PrepareSendFloats(float *values, unsigned char count);

// 内部函数: 浮点数发送回调函数
unsigned char SPI_FloatTxCallback(void);

#endif // __SPI_MULTIDEVICE_H__
