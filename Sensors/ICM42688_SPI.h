#ifndef __ICM42688_SPI_H__
#define __ICM42688_SPI_H__

#include "Gyroscope.h"

// ICM42688-P寄存器地址定义 - 将常量定义移至头文件
#define ICM42688_WHOAMI           0x75
#define ICM42688_PWR_MGMT0        0x4E
#define ICM42688_GYRO_CONFIG0     0x4F
#define ICM42688_ACCEL_CONFIG0    0x50
#define ICM42688_GYRO_DATA_X1     0x25
#define ICM42688_GYRO_DATA_X0     0x26
#define ICM42688_GYRO_DATA_Y1     0x27
#define ICM42688_GYRO_DATA_Y0     0x28
#define ICM42688_GYRO_DATA_Z1     0x29
#define ICM42688_GYRO_DATA_Z0     0x2A
#define ICM42688_ACCEL_DATA_X1    0x1F
#define ICM42688_ACCEL_DATA_X0    0x20
#define ICM42688_ACCEL_DATA_Y1    0x21
#define ICM42688_ACCEL_DATA_Y0    0x22
#define ICM42688_ACCEL_DATA_Z1    0x23
#define ICM42688_ACCEL_DATA_Z0    0x24

// 初始化ICM-42688陀螺仪SPI接口
int ICM42688_SPI_Init(void);

// 从ICM-42688读取单个寄存器
unsigned char ICM42688_ReadReg(unsigned char);

// 向ICM-42688写入单个寄存器
void ICM42688_WriteReg(unsigned char, unsigned char);

// 从ICM-42688读取原始传感器数据
int ICM42688_ReadSensorData(icm426888_data_t *);

#endif // __ICM42688_SPI_H__
