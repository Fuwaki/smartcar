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

#define SPI_CLOCK_DIV2 0x00 // 时钟分频2
#define SPI_CLOCK_DIV4 0x01 // 时钟分频4
#define SPI_CLOCK_DIV8 0x02 // 时钟分频8
#define SPI_CLOCK_DIV16 0x03 // 时钟分频16
#define SPI_CLOCK_DIV32 0x04 // 时钟分频32

// 从设备定义结构体
typedef struct
{
    unsigned char cs_port;   // CS引脚所在端口 (0:P0, 1:P1, 2:P2, 3:P3, 4:P4, 5:P5, 6:P6, 7:P7)
    unsigned char cs_pin;    // CS引脚编号 (0-7)
    unsigned char mode;      // SPI模式 (0-3)
    unsigned char clock_div; // 时钟分频 (2/4/8/16/32/64/128)
} spi_slave_config_t;

typedef struct 
{
    #pragma region GPS数据
    float GPS_Raw_X;//纬度
    float GPS_Raw_Y;//经度
    float GPS_Nature_X;
    float GPS_Nature_Y;
    float GPS_Heading;
    float GPS_Speed;
    #pragma endregion GPS数据

    #pragma region IMU数据
    float IMU_Acc_X;
    float IMU_Acc_Y;
    float IMU_Acc_Z;
    float IMU_Gyro_X; //dps
    float IMU_Gyro_Y;
    float IMU_Gyro_Z;
    float IMU_Temperature;
    #pragma endregion IMU数据

    #pragma region Mag数据
    float Mag_Raw_X;
    float Mag_Raw_Y;
    float Mag_Raw_Z;
    float Mag_Adujsted_X;
    float Mag_Adujsted_Y;
    float Mag_Adujsted_Z;
    float Mag_Heading;
    #pragma endregion Mag数据

    #pragma region encoder数据
    float Encoder_Speed;
    #pragma endregion encoder数据
}SENSOR_DATA; //发送结构体
extern SENSOR_DATA senddata; //声明结构体变量

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



// SENSOR_DATA结构体传输相关函数
void SPI_SlaveStartSendSensorData(SENSOR_DATA* connectData);
unsigned char SPI_SlaveSendSensorDataByte(void);
bit SPI_IsStructTransmissionActive(void);
void SPI_CancelStructTransmission(void);
void SPI_SlaveModeMessageUpdater(SENSOR_DATA* connectData);
#endif // __SPI_MULTIDEVICE_H__
