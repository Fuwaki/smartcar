#include <AI8051U.H>
#include <stdio.h>
#include "SPI_MultiDevice.h"
#include "Gyroscope.h"
#include "ICM42688_SPI.h"

// 陀螺仪SPI从设备ID
static unsigned char icm42688_slave_id = 0xFF;

/**
 * @brief 初始化ICM-42688陀螺仪SPI接口
 * @return 0 - 成功, 非0 - 失败
 */
int ICM42688_SPI_Init(void)
{
    spi_slave_config_t slave_config;
    
    // 配置SPI从设备参数
    slave_config.cs_port = 1;           // 假设片选连接到P1口
    slave_config.cs_pin = 4;            // 假设片选为P1.4
    slave_config.mode = SPI_MODE0;      // ICM-42688使用SPI模式0
    slave_config.clock_div = 8;         // SPI时钟分频
    
    // 注册SPI从设备
    icm42688_slave_id = SPI_RegisterSlave(&slave_config);
    if(icm42688_slave_id == 0xFF) {
        return -1; // 注册失败
    }
    
    // 读取WHO_AM_I寄存器确认设备ID
    unsigned char who_am_i = ICM42688_ReadReg(ICM42688_WHOAMI);
    if(who_am_i != 0x47) {  // ICM-42688的WHO_AM_I值应为0x47
        return -2; // 设备ID不匹配
    }
    
    // 配置陀螺仪
    // 退出睡眠模式，启用加速度计和陀螺仪
    ICM42688_WriteReg(ICM42688_PWR_MGMT0, 0x0F);
    
    // 配置陀螺仪量程(±2000dps)和ODR(1kHz)
    ICM42688_WriteReg(ICM42688_GYRO_CONFIG0, 0x06);
    
    // 配置加速度计量程(±16g)和ODR(1kHz)
    ICM42688_WriteReg(ICM42688_ACCEL_CONFIG0, 0x06);
    
    return 0; // 初始化成功
}

/**
 * @brief 从ICM-42688读取单个寄存器
 * @param reg_addr 寄存器地址
 * @return 寄存器值
 */
unsigned char ICM42688_ReadReg(unsigned char reg_addr)
{
    return SPI_ReadRegister(icm42688_slave_id, reg_addr);
}

/**
 * @brief 向ICM-42688写入单个寄存器
 * @param reg_addr 寄存器地址
 * @param value 寄存器值
 */
void ICM42688_WriteReg(unsigned char reg_addr, unsigned char value)
{
    SPI_WriteRegister(icm42688_slave_id, reg_addr, value);
}

/**
 * @brief 从ICM-42688读取原始传感器数据
 * @param data 指向存储数据的结构体
 * @return 0 - 成功, 非0 - 失败
 */
int ICM42688_ReadSensorData(icm426888_data_t *data)
{
    unsigned char buffer[12];
    int raw_accel_x, raw_accel_y, raw_accel_z;
    int raw_gyro_x, raw_gyro_y, raw_gyro_z;
    float accel_scale, gyro_scale;
    
    if(data == NULL) {
        return -1;
    }
    
    // 从ICM-42688连续读取12个字节的传感器数据
    SPI_ReadMultiRegisters(icm42688_slave_id, ICM42688_ACCEL_DATA_X1, buffer, 12);
    
    // 合并传感器数据（16位有符号）
    raw_accel_x = ((int)buffer[0] << 8) | buffer[1];
    raw_accel_y = ((int)buffer[2] << 8) | buffer[3];
    raw_accel_z = ((int)buffer[4] << 8) | buffer[5];
    raw_gyro_x = ((int)buffer[6] << 8) | buffer[7];
    raw_gyro_y = ((int)buffer[8] << 8) | buffer[9];
    raw_gyro_z = ((int)buffer[10] << 8) | buffer[11];
    
    // 根据配置的量程计算转换因子
    accel_scale = 16.0f / 32768.0f * 1000.0f; // 转换为 mg (±16g量程)
    gyro_scale = 2000.0f / 32768.0f;          // 转换为 degrees/s (±2000dps量程)
    
    // 转换为物理量
    data->accel_x = raw_accel_x * accel_scale;
    data->accel_y = raw_accel_y * accel_scale;
    data->accel_z = raw_accel_z * accel_scale;
    
    data->gyro_x = raw_gyro_x * gyro_scale;
    data->gyro_y = raw_gyro_y * gyro_scale;
    data->gyro_z = raw_gyro_z * gyro_scale;
    
    // 读取温度数据
    unsigned char temp_buffer[2];
    SPI_ReadMultiRegisters(icm42688_slave_id, 0x1D, temp_buffer, 2);
    int raw_temp = ((int)temp_buffer[0] << 8) | temp_buffer[1];
    
    // 温度转换（根据数据手册）
    data->temperature = (raw_temp / 132.48f) + 25.0f;
    
    return 0;
}
