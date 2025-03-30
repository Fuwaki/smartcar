#include <intrins.h>
#include <stdio.h>
#include "Gyroscope.h"
#include "SPI_MultiDevice.h"

// 当前配置的陀螺仪和加速度计范围
static gyro_range_t current_gyro_range = GYRO_RANGE_2000_DPS;
static accel_range_t current_accel_range = ACCEL_RANGE_16G;
static unsigned char icm42688_spi_id = 0xFF;

void Gyrp_Delay(void)	//@40.000MHz 1ms延时
{
	unsigned long edata i;

	_nop_();
	_nop_();
	_nop_();
	i = 9998UL;
	while (i) i--;
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
    icm42688_config.cs_port = 1;
    icm42688_config.cs_pin = 6;
    icm42688_config.mode = SPI_MODE3;   // ICM42688使用SPI模式3 (CPOL=1, CPHA=1)
    icm42688_config.clock_div = 0x00;   // SPI时钟速率设置，根据需要调整

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

    // 配置电源管理，使能加速度计和陀螺仪，低噪声模式
    ICM42688_WriteRegister(icm42688_spi_id, ICM42688_PWR_MGMT0,
                           ICM42688_PWR_MGMT0_ACCEL_MODE_LN | ICM42688_PWR_MGMT0_GYRO_MODE_LN);
    
    // 等待传感器启动（按需调整延迟时间）
    Gyrp_Delay();

    // 设置默认范围
    //TODO: 选择合适的量程
    ICM42688_SetGyroRange(GYRO_RANGE_15_625_DPS);     
    ICM42688_SetAccelRange(ACCEL_RANGE_2G);

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
    unsigned char buffer[14]; // 7个16位值：加速度(3)、温度、陀螺仪(3)

    if (dataf == NULL)
    {
        return;
    }

    // 一次性读取所有数据 (从ACCEL_DATA_X1到GYRO_DATA_Z0)
    ICM42688_ReadMultiRegisters(icm42688_spi_id, ICM42688_ACCEL_DATA_X1, buffer, 14);

    // 合并高低字节（大端格式）
    dataf->accel_x = (buffer[0] << 8) | buffer[1];
    dataf->accel_y = (buffer[2] << 8) | buffer[3];
    dataf->accel_z = (buffer[4] << 8) | buffer[5];

    dataf->temp = (buffer[6] << 8) | buffer[7];

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
    dataf->gyro_z_dps = ICM42688_GyroConvert(dataf->gyro_z, current_gyro_range);
    
    dataf->temp_c = ICM42688_GetTemperature(dataf->temp);
}

// 软件复位ICM42688-P
void ICM42688_Reset()
{
    // 写入设备配置寄存器，设置软件复位位
    ICM42688_WriteRegister(icm42688_spi_id , ICM42688_DEVICE_CONFIG, 0x01);

    // 等待复位完成（通常需要几毫秒）
    Gyrp_Delay();
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

void Gyro_Updater()
{
    static icm42688_data_t sensor_data;
    // 在这里处理传感器数据

    // static icm42688_data_t filtered_data;
    // static kalman_filter_t kf_accel_x, kf_accel_y, kf_accel_z;
    // static kalman_filter_t kf_gyro_x, kf_gyro_y, kf_gyro_z;
    // static unsigned char is_initialized = 0;
    
    // // 初始化卡尔曼滤波器（仅在第一次调用时）
    // if (!is_initialized) {
    //     init_gyro_kalman_filters(&kf_accel_x, &kf_accel_y, &kf_accel_z,
    //                          &kf_gyro_x, &kf_gyro_y, &kf_gyro_z);
    //     is_initialized = 1;
    // }
    
    // 读取传感器数据
    ICM42688_ReadSensorData(&sensor_data);
    
    // 应用卡尔曼滤波
    // apply_kalman_filter(&sensor_data, &filtered_data, 
    //                     &kf_accel_x, &kf_accel_y, &kf_accel_z,
    //                     &kf_gyro_x, &kf_gyro_y, &kf_gyro_z);
    

    //比如说FK u;
}


#pragma region KalmanFilter// 卡尔曼滤波器结构体
/**
 * @brief 初始化卡尔曼滤波器
 * @param filter 滤波器结构体指针
 * @param Q 过程噪声协方差
 * @param R 测量噪声协方差
 * @param P_initial 初始估计误差协方差
 * @param x_initial 初始状态估计
 */
void kalman_init(kalman_filter_t *filter, float Q, float R, float P_initial, float x_initial) 
{
    filter->x = x_initial;
    filter->P = P_initial;
    filter->Q = Q;
    filter->R = R;
    filter->K = 0;
}

/**
 * @brief 卡尔曼滤波更新步骤
 * @param filter 滤波器结构体指针
 * @param measurement 当前测量值
 * @return 滤波后的估计值
 */
float kalman_update(kalman_filter_t *filter, float measurement) 
{
    // 预测步骤
    // x = x (状态预测，简化模型下保持不变)
    filter->P = filter->P + filter->Q;
    
    // 更新步骤
    filter->K = filter->P / (filter->P + filter->R);
    filter->x = filter->x + filter->K * (measurement - filter->x);
    filter->P = (1 - filter->K) * filter->P;
    
    return filter->x;
}

/**
 * @brief 初始化所有陀螺仪数据的卡尔曼滤波器
 * @param kf_accel_x,kf_accel_y,kf_accel_z 加速度数据的滤波器
 * @param kf_gyro_x,kf_gyro_y,kf_gyro_z 陀螺仪数据的滤波器
 */
void init_gyro_kalman_filters(kalman_filter_t *kf_accel_x, kalman_filter_t *kf_accel_y, kalman_filter_t *kf_accel_z,
    kalman_filter_t *kf_gyro_x, kalman_filter_t *kf_gyro_y, kalman_filter_t *kf_gyro_z)
{
    // 初始化加速度计滤波器
    // 参数可根据实际应用调整：Q(过程噪声), R(测量噪声), 初始P, 初始x
    kalman_init(kf_accel_x, 0.01f, 0.1f, 1.0f, 0.0f);
    kalman_init(kf_accel_y, 0.01f, 0.1f, 1.0f, 0.0f);
    kalman_init(kf_accel_z, 0.01f, 0.1f, 1.0f, 0.0f);

    // 初始化陀螺仪滤波器
    kalman_init(kf_gyro_x, 0.003f, 0.03f, 1.0f, 0.0f);
    kalman_init(kf_gyro_y, 0.003f, 0.03f, 1.0f, 0.0f);
    kalman_init(kf_gyro_z, 0.003f, 0.03f, 1.0f, 0.0f);
}

/**
 * @brief 对陀螺仪和加速度数据应用卡尔曼滤波
 * @param raw_data 原始传感器数据
 * @param filtered_data 滤波后的数据
 * @param kf_accel_x,kf_accel_y,kf_accel_z 加速度数据的滤波器
 * @param kf_gyro_x,kf_gyro_y,kf_gyro_z 陀螺仪数据的滤波器
 */
void apply_kalman_filter(icm42688_data_t *raw_data, icm42688_data_t *filtered_data, 
    kalman_filter_t *kf_accel_x, kalman_filter_t *kf_accel_y, kalman_filter_t *kf_accel_z,
    kalman_filter_t *kf_gyro_x, kalman_filter_t *kf_gyro_y, kalman_filter_t *kf_gyro_z)
{
    // 复制原始数据，以便处理转换后的值
    *filtered_data = *raw_data;
    
    // 更新加速度数据
    filtered_data->accel_x_g = kalman_update(kf_accel_x, raw_data->accel_x_g);
    filtered_data->accel_y_g = kalman_update(kf_accel_y, raw_data->accel_y_g);
    filtered_data->accel_z_g = kalman_update(kf_accel_z, raw_data->accel_z_g);

    // 更新陀螺仪数据
    filtered_data->gyro_x_dps = kalman_update(kf_gyro_x, raw_data->gyro_x_dps);
    filtered_data->gyro_y_dps = kalman_update(kf_gyro_y, raw_data->gyro_y_dps);
    filtered_data->gyro_z_dps = kalman_update(kf_gyro_z, raw_data->gyro_z_dps);
}
#pragma endregion

//huh, maybe that's shall be end.Fuwaki Ur shall be happy to see this.
//I'm brain fucked by this code. I'm not sure if it's right or wrong. I'm not sure if it's useful or not.
//I'm not sure if it's a good idea to use this code or not. I'm not sure if it's a good idea to use this code or not.
//EDITED by UNIKOZERA!
//Ciallo! I'm UNIKOZERA