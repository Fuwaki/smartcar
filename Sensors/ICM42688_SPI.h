#ifndef __ICM42688_SPI_H__
#define __ICM42688_SPI_H__

#include "Gyroscope.h"

    // 初始化ICM-42688陀螺仪SPI接口
    int ICM42688_SPI_Init(void);

    // 从ICM-42688读取单个寄存器
    unsigned char ICM42688_ReadReg(unsigned char reg_addr);

    // 向ICM-42688写入单个寄存器
    void ICM42688_WriteReg(unsigned char reg_addr, unsigned char value);

    // 从ICM-42688读取原始传感器数据
    int ICM42688_ReadSensorData(icm426888_data_t *data);

#endif // __ICM42688_SPI_H__
