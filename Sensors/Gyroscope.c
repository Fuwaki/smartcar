#include <intrins.h>
#include <stdio.h>
#include "GPS.h"
#include "Gyroscope.h"
#include "SPI_MultiDevice.h"

// 当前配置的陀螺仪和加速度计范围
static gyro_range_t current_gyro_range = GYRO_RANGE_125_DPS;
static accel_range_t current_accel_range = ACCEL_RANGE_4G;
static unsigned char icm42688_spi_id = 0xFF;

icm42688_data_t gyro_data;

static float gyro_z_offset = 0.0f;
static bit gyro_z_calibrated = 0;  // 校准标志位

typedef struct
{
    float x;                   // 状态估计值（过滤后的DPS值）
    float P;                   // 估计误差协方差
    float Q;                   // 过程噪声协方差（系统噪声）
    float R;                   // 测量噪声协方差（传感器噪声）
    float K;                   // 卡尔曼增益
    unsigned char initialized; // 初始化标志
} DPS_KalmanFilter;
DPS_KalmanFilter gyro_x_filter, gyro_y_filter, gyro_z_filter;

bit allowUpdate = 0; // 允许陀螺仪更新航向角数据

#pragma region Kalman_Filter
// 初始化DPS卡尔曼滤波器
void DPS_Kalman_Init(DPS_KalmanFilter *filter, float process_noise, float measurement_noise)
{
    filter->x = 0.0f;              // 初始状态估计为0
    filter->P = 1.0f;              // 初始估计误差协方差较大，表示不确定性
    filter->Q = process_noise;     // 过程噪声协方差 (推荐值: 0.001 - 0.01)
    filter->R = measurement_noise; // 测量噪声协方差 (推荐值: 0.01 - 0.1)
    filter->K = 0.0f;              // 初始卡尔曼增益
    filter->initialized = 0;       // 未初始化状态
}

// 卡尔曼滤波器更新 - 处理陀螺仪DPS值
float DPS_Kalman_Update(DPS_KalmanFilter *filter, float measurement)
{
    // 第一次使用时直接初始化状态为测量值
    if (!filter->initialized)
    {
        filter->x = measurement;
        filter->initialized = 1;
        return filter->x;
    }

    // 预测步骤 - 状态预测(简化模型假设状态不变)
    // 更新误差协方差
    filter->P = filter->P + filter->Q;

    // 更新步骤
    // 计算卡尔曼增益
    filter->K = filter->P / (filter->P + filter->R);

    // 用测量值更新状态估计
    filter->x = filter->x + filter->K * (measurement - filter->x);

    // 更新误差协方差
    filter->P = (1.0f - filter->K) * filter->P;

    return filter->x;
}

// 重置滤波器
void DPS_Kalman_Reset(DPS_KalmanFilter *filter)
{
    filter->x = 0.0f;
    filter->P = 1.0f;
    filter->K = 0.0f;
    filter->initialized = 0;
}
#pragma endregion

void Gyro_Delay(void) //@40.000MHz 10ms延时
{
    unsigned long edata i;

    _nop_();
    _nop_();
    i = 99998UL;
    while (i)
        i--;
}

// 读取ICM42688-P寄存器
unsigned char ICM42688_ReadRegister(unsigned char slave_id, unsigned char reg_addr)
{
    return SPI_ReadRegister(slave_id, reg_addr | ICM42688_READ_FLAG);
}

// 写入ICM42688-P寄存器
void ICM42688_WriteRegister(unsigned char slave_id, unsigned char reg_addr, unsigned char value)
{
    SPI_WriteRegister(slave_id, reg_addr, value);
}

// 读取多个寄存器
void ICM42688_ReadMultiRegisters(unsigned char slave_id, unsigned char start_addr,
                                 unsigned char *buffer, unsigned int count)
{
    SPI_ReadMultiRegisters(slave_id, start_addr | ICM42688_READ_FLAG, buffer, count);
}

// 初始化ICM42688-P传感器
unsigned char ICM42688_Init()
{
    unsigned char who_am_i;

    spi_slave_config_t icm42688_config;
    icm42688_config.cs_port = 0;
    icm42688_config.cs_pin = 2;
    icm42688_config.mode = SPI_MODE0;           // ICM42688使用SPI模式0
    icm42688_config.clock_div = SPI_CLOCK_DIV4; // SPI时钟速率设置，根据需要调整

    // 注册ICM42688-P为SPI从设备
    icm42688_spi_id = SPI_RegisterSlave(&icm42688_config);
    // 软件复位
    ICM42688_Reset(icm42688_spi_id);

    // 检查设备ID
    who_am_i = ICM42688_ReadRegister(icm42688_spi_id, ICM42688_WHO_AM_I);
    if (who_am_i != ICM42688_WHO_AM_I_VALUE)
    {
        return 1; // 初始化失败，设备ID不匹配
    }
    // 初始化滤波器 - 参数可根据实际噪声情况调整
    // 第一个参数: 过程噪声(越大响应越快但越不稳定)
    // 第二个参数: 测量噪声(越大滤波越强但滞后越明显)
    DPS_Kalman_Init(&gyro_x_filter, 0.0015f, 0.05f);
    DPS_Kalman_Init(&gyro_y_filter, 0.0015f, 0.05f);
    DPS_Kalman_Init(&gyro_z_filter, 0.0015f, 0.05f);

    // 配置电源管理，使能加速度计和陀螺仪，低噪声模式
    ICM42688_WriteRegister(icm42688_spi_id, ICM42688_PWR_MGMT0,
                           ICM42688_PWR_MGMT0_ACCEL_MODE_LN | ICM42688_PWR_MGMT0_GYRO_MODE_LN);

    // 等待传感器启动（按需调整延迟时间）
    Gyro_Delay();

    // 设置默认范围
    // TODO: 选择合适的量程
    ICM42688_SetGyroRange(GYRO_RANGE_125_DPS);
    ICM42688_SetAccelRange(ACCEL_RANGE_4G);
    // ICM42688_CalibrateGyroZ();

    gyro_data.true_yaw_angle = 0.0f; // 初始化偏航角为0.0f

    return 0; // 初始化成功
}

// 设置陀螺仪量程
void ICM42688_SetGyroRange(gyro_range_t range)
{
    unsigned char config;

    if (range > GYRO_RANGE_15_625_DPS)
    {
        range = GYRO_RANGE_15_625_DPS; // 非法范围，设为默认值
    }

    config = ICM42688_ReadRegister(icm42688_spi_id, ICM42688_GYRO_CONFIG0);
    config = (config & 0xF8) | (range & 0x07); // 保留高5位，修改低3位
    ICM42688_WriteRegister(icm42688_spi_id, ICM42688_GYRO_CONFIG0, config);

    current_gyro_range = range;
}

// 设置加速度计量程
void ICM42688_SetAccelRange(accel_range_t range)
{
    unsigned char config;

    if (range > ACCEL_RANGE_2G)
    {
        range = ACCEL_RANGE_2G; // 非法范围，设为默认值
    }

    config = ICM42688_ReadRegister(icm42688_spi_id, ICM42688_ACCEL_CONFIG0);
    config = (config & 0xFC) | (range & 0x03); // 保留高6位，修改低2位
    ICM42688_WriteRegister(icm42688_spi_id, ICM42688_ACCEL_CONFIG0, config);

    current_accel_range = range;
}

// 读取传感器原始数据
void ICM42688_ReadSensorData(icm42688_data_t *dataf)
{
    unsigned char buffer[16]; // 8个16位值：温度(1)、加速度(3)、陀螺仪(3)

    if (dataf == NULL)
    {
        return;
    }

    // 一次性读取所有数据 (从TEMP_DATA1到GYRO_DATA_Z0)
    ICM42688_ReadMultiRegisters(icm42688_spi_id, ICM42688_TEMP_DATA1, buffer, 16);

    // 合并高低字节（大端格式）
    dataf->temp = (buffer[0] << 8) | buffer[1];
    dataf->accel_x = (buffer[2] << 8) | buffer[3];
    dataf->accel_y = (buffer[4] << 8) | buffer[5];
    dataf->accel_z = (buffer[6] << 8) | buffer[7];
    dataf->gyro_x = (buffer[8] << 8) | buffer[9];
    dataf->gyro_y = (buffer[10] << 8) | buffer[11];
    dataf->gyro_z = (buffer[12] << 8) | buffer[13];

    /*如果你想用卡尔曼滤波器，就在这里插入卡尔曼滤波器的代码
        Put you code here!
    */

    // 计算转换后的物理单位数据
    dataf->accel_x_g = ICM42688_AccelConvert(dataf->accel_x, current_accel_range);
    dataf->accel_y_g = ICM42688_AccelConvert(dataf->accel_y, current_accel_range);
    dataf->accel_z_g = ICM42688_AccelConvert(dataf->accel_z, current_accel_range);

    dataf->gyro_x_dps = ICM42688_GyroConvert(dataf->gyro_x, current_gyro_range);
    dataf->gyro_y_dps = ICM42688_GyroConvert(dataf->gyro_y, current_gyro_range);
    dataf->gyro_z_dps = ICM42688_GyroConvert(dataf->gyro_z, current_gyro_range) - 0.0191f;

    dataf->temp_c = ICM42688_GetTemperature(dataf->temp);
}

// 软件复位ICM42688-P
void ICM42688_Reset()
{
    // 写入设备配置寄存器，设置软件复位位
    ICM42688_WriteRegister(icm42688_spi_id, ICM42688_DEVICE_CONFIG, 0x01);

    // 等待复位完成（通常需要几毫秒）
    Gyro_Delay();
}

// 检查传感器通信是否正常
unsigned char ICM42688_TestConnection()
{
    unsigned char who_am_i = ICM42688_ReadRegister(icm42688_spi_id, ICM42688_WHO_AM_I);
    return (who_am_i == ICM42688_WHO_AM_I_VALUE) ? 0 : 1;
}

// 获取转换后的温度值（摄氏度）
// 根据ICM42688规格书，温度转换公式为：
// Temp(°C) = (RawTemp / 132.48) + 25
float ICM42688_GetTemperature(int raw_temp)
{
    return (raw_temp / 132.48f) + 25.0f;
}

// 根据当前设置的范围转换陀螺仪原始数据为度/秒
float ICM42688_GyroConvert(int raw_gyro, gyro_range_t range)
{
    float dps_per_lsb;

    // 根据不同的量程范围确定每LSB对应的度/秒
    switch (range)
    {
    case GYRO_RANGE_2000_DPS:
        dps_per_lsb = 2000.0f / 32768.0f;
        break;
    case GYRO_RANGE_1000_DPS:
        dps_per_lsb = 1000.0f / 32768.0f;
        break;
    case GYRO_RANGE_500_DPS:
        dps_per_lsb = 500.0f / 32768.0f;
        break;
    case GYRO_RANGE_250_DPS:
        dps_per_lsb = 250.0f / 32768.0f;
        break;
    case GYRO_RANGE_125_DPS:
        dps_per_lsb = 125.0f / 32768.0f;
        break;
    case GYRO_RANGE_62_5_DPS:
        dps_per_lsb = 62.5f / 32768.0f;
        break;
    case GYRO_RANGE_31_25_DPS:
        dps_per_lsb = 31.25f / 32768.0f;
        break;
    case GYRO_RANGE_15_625_DPS:
        dps_per_lsb = 15.625f / 32768.0f;
        break;
    default:
        dps_per_lsb = 2000.0f / 32768.0f;
        break;
    }

    return raw_gyro * dps_per_lsb;
}

// 根据当前设置的范围转换加速度计原始数据为G
float ICM42688_AccelConvert(int raw_accel, accel_range_t range)
{
    float g_per_lsb;

    // 根据不同的量程范围确定每LSB对应的G值
    switch (range)
    {
    case ACCEL_RANGE_16G:
        g_per_lsb = 16.0f / 32768.0f;
        break;
    case ACCEL_RANGE_8G:
        g_per_lsb = 8.0f / 32768.0f;
        break;
    case ACCEL_RANGE_4G:
        g_per_lsb = 4.0f / 32768.0f;
        break;
    case ACCEL_RANGE_2G:
        g_per_lsb = 2.0f / 32768.0f;
        break;
    default:
        g_per_lsb = 16.0f / 32768.0f;
        break;
    }

    return raw_accel * g_per_lsb;
}

// void ICM42688_CalibrateGyroZ()
// {
//     unsigned int i;
//     unsigned char samples = 100; // 采样数量
//     float filtered_value; 
//     float sum = 0.0f;
//     icm42688_data_t temp_data;
    
//     // 等待传感器稳定
//     for (i = 0; i < 10; i++)
//     {
//         Gyro_Delay();  // 假设此函数提供约10ms延时
//     }
    
//     // 采集多个样本并计算平均值
//     for (i = 0; i < samples; i++) 
//     {
//         // 读取原始传感器数据
//         ICM42688_ReadSensorData(&temp_data);
        
//         // 应用卡尔曼滤波但不减去任何偏移量
//         filtered_value = DPS_Kalman_Update(&gyro_z_filter, temp_data.gyro_z_dps);
//         // 累加滤波后的值
//         sum += filtered_value;
        
//         Gyro_Delay();  // 延时以允许传感器稳定
//     }
    
//     // 计算平均偏移量
//     gyro_z_offset = sum / samples;
//     gyro_z_calibrated = 1;

// }

void Gyro_Updater()
{
    // 读取传感器数据
    ICM42688_ReadSensorData(&gyro_data);
    // 使用卡尔曼滤波器处理DPS数据 - 0.01796 、、- 0.02796
    gyro_data.gyro_x_dps_kf = DPS_Kalman_Update(&gyro_x_filter, gyro_data.gyro_x_dps);
    gyro_data.gyro_y_dps_kf = DPS_Kalman_Update(&gyro_y_filter, gyro_data.gyro_y_dps);
    // 使用动态校准的偏移量而非固定值
    // if (gyro_z_calibrated) 
    // {
    //     gyro_data.gyro_z_dps_kf = DPS_Kalman_Update(&gyro_z_filter, gyro_data.gyro_z_dps) - gyro_z_offset;
    // } 
    // else 
    // {
    gyro_data.gyro_z_dps_kf = DPS_Kalman_Update(&gyro_z_filter, gyro_data.gyro_z_dps);
    // }
}

// huh, maybe that's shall be end.Fuwaki Ur shall be happy to see this.
// I'm brain fucked by this code. I'm not sure if it's right or wrong. I'm not sure if it's useful or not.
// I'm not sure if it's a good idea to use this code or not. I'm not sure if it's a good idea to use this code or not.
// EDITED by UNIKOZERA!
// Ciallo! I'm UNIKOZERA

#pragma region yaw_Calculation //!yaw的陀螺仪累加已经在timer中实现
void yaw_angle_init()
{
    // static float yaw_angle_offset = 0.0f; // 偏航角偏移量
    // static float yaw_angle_sum = 0.0f;    // 偏航角累加值
    // static unsigned char i = 0;
    // if (rmc_data.valid && i <= 50 && !allowUpdate)
    // {
    //     yaw_angle_sum += rmc_data.course;
    //     i++;
    // }
    // else if (rmc_data.valid && i > 50 && !allowUpdate)
    // {
    //     yaw_angle_offset = yaw_angle_sum / 50.0f; // 计算偏航角偏移量
    //     gyro_data.true_yaw_angle = yaw_angle_offset;
    //     allowUpdate = 1; // 允许更新航向角数据
    // } // 成功初始化
    if (!allowUpdate) // 如果允许更新航向角数据，则返回
    {
        gyro_data.true_yaw_angle = 60;
        allowUpdate = 1; // 禁止更新航向角数据
    }
}
#pragma endregion