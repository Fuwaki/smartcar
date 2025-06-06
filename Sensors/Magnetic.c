#include <AI8051U.H>
#include <intrins.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "Magnetic.h"
#include "SPI_MultiDevice.h"

#define PI 3.14159265358979323846 // 这么长怎么你了!
#define LIS3MDL_READ 0xC0  // 读操作最高位为1
#define LIS3MDL_WRITE 0x00 // 写操作最高位为0
#define LIS3MDL_MS 0x40    // 多字节读取模式标识位
unsigned char current_scale = LIS3MDL_FS_4GAUSS;
unsigned char lis3mdl_spi_id = 0xFF; //初始值为0xFF表示未注册

typedef struct
{
    float x;                   // 状态估计值（过滤后的DPS值）
    float P;                   // 估计误差协方差
    float Q;                   // 过程噪声协方差（系统噪声）
    float R;                   // 测量噪声协方差（传感器噪声）
    float K;                   // 卡尔曼增益
    unsigned char initialized; // 初始化标志
} DPS_KalmanFilter;
DPS_KalmanFilter mag_x_filter, mag_y_filter, mag_z_filter;

// 磁力计校准参数
typedef struct
{
    float x_offset;              // X轴硬铁偏移
    float y_offset;              // Y轴硬铁偏移
    float z_offset;              // Z轴硬铁偏移
    float x_scale;               // X轴软铁比例因子
    float y_scale;               // Y轴软铁比例因子
    float z_scale;               // Z轴软铁比例因子
    unsigned char is_calibrated; // 是否已校准标志
} MagCalibration;

// 全局校准参数
static MagCalibration mag_calibration = {0};
MagneticData mag_data;

// 用于校准采样的缓冲区
#define MAG_CALIB_SAMPLES 100
unsigned int sample_count = 0;
static MagneticData mag_samples[MAG_CALIB_SAMPLES];
static unsigned char calibration_in_progress = 0;

#pragma region Kalman_Filter
// 初始化DPS卡尔曼滤波器
void Mag_Kalman_Init(DPS_KalmanFilter *filter, float process_noise, float measurement_noise)
{
    filter->x = 0.0f;              // 初始状态估计为0
    filter->P = 1.0f;              // 初始估计误差协方差较大，表示不确定性
    filter->Q = process_noise;     // 过程噪声协方差 (推荐值: 0.001 - 0.01)
    filter->R = measurement_noise; // 测量噪声协方差 (推荐值: 0.01 - 0.1)
    filter->K = 0.0f;              // 初始卡尔曼增益
    filter->initialized = 0;       // 未初始化状态
}

// 卡尔曼滤波器更新 - 处理陀螺仪DPS值
float Mag_Kalman_Update(DPS_KalmanFilter *filter, float measurement)
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
void Mag_Kalman_Reset(DPS_KalmanFilter *filter)
{
    filter->x = 0.0f;
    filter->P = 1.0f;
    filter->K = 0.0f;
    filter->initialized = 0;
}
#pragma endregion


void Mag_Delay(void) //1ms
{
    unsigned long edata i;

    _nop_();
    _nop_();
    _nop_();
    i = 9998UL;
    while (i)
        i--;
}

// 初始化磁力计校准参数
void LIS3MDL_InitCalibration(void)
{
    //TODO: 使用默认的校准参数 还是别用Init磁场计算法了

    // 默认硬铁校正参数（理想情况下应通过校准过程确定）
    mag_calibration.x_offset = 0.0f;
    mag_calibration.y_offset = 0.0f;
    mag_calibration.z_offset = 0.0f;

    // 默认软铁校正参数（理想情况下应通过校准过程确定）
    mag_calibration.x_scale = 1.0f;
    mag_calibration.y_scale = 1.0f;
    mag_calibration.z_scale = 1.0f;

    mag_calibration.is_calibrated = 0; // 标记为未校准
}

// LIS3MDL初始化函数
unsigned char LIS3MDL_Init(void)
{
    unsigned char device_id;
    // 配置SPI从设备
    spi_slave_config_t lis3mdl_config;
    lis3mdl_config.cs_port = 4;
    lis3mdl_config.cs_pin = 2;
    lis3mdl_config.mode = SPI_MODE0; // LIS3MDL使用SPI模式0
    lis3mdl_config.clock_div = SPI_CLOCK_DIV4; // SPI时钟分频4

    // 注册LIS3MDL为SPI从设备
    lis3mdl_spi_id = SPI_RegisterSlave(&lis3mdl_config);

    if (lis3mdl_spi_id == 0xFF)
    {
        // 注册失败处理
        return 0;
    }

    Mag_Kalman_Init(&mag_x_filter, 0.0015f, 0.05f);
    Mag_Kalman_Init(&mag_y_filter, 0.0015f, 0.05f);
    Mag_Kalman_Init(&mag_z_filter, 0.0015f, 0.05f);
    // 适当延时以确保SPI总线稳定
    Mag_Delay();

    //复位设备
    LIS3MDL_WriteReg(LIS3MDL_CTRL_REG2, 0x0C); // 软复位

    Mag_Delay(); // 等待复位完成

    // 验证设备ID
    device_id = LIS3MDL_ReadReg(LIS3MDL_WHO_AM_I);
    if (device_id != 0x3D)
    {
        return 1; //这就是错误处理代码qwq
    }

    //配置传感器
    // CTRL_REG1: 温度传感器使能, XY轴高性能模式, 80Hz输出速率
    LIS3MDL_WriteReg(LIS3MDL_CTRL_REG1, 0xFE);
    Mag_Delay(); // 等待配置完成

    // CTRL_REG2: 设置量程为±4高斯
    LIS3MDL_WriteReg(LIS3MDL_CTRL_REG2, LIS3MDL_FS_4GAUSS);
    
    current_scale = LIS3MDL_FS_4GAUSS;
    Mag_Delay(); // 等待配置完成
    // CTRL_REG3: 连续转换模式
    LIS3MDL_WriteReg(LIS3MDL_CTRL_REG3, 0x00);
    Mag_Delay(); // 等待配置完成
    // CTRL_REG4: Z轴高性能模式, 大端数据
    LIS3MDL_WriteReg(LIS3MDL_CTRL_REG4, 0x0C);
    Mag_Delay(); // 等待配置完成
    // CTRL_REG5: 快速读取模式
    LIS3MDL_WriteReg(LIS3MDL_CTRL_REG5, 0x40);
    Mag_Delay(); // 等待配置完成
    // 初始化校准参数
    // LIS3MDL_InitCalibration();
    return 2;
}

// 读取LIS3MDL寄存器
unsigned char LIS3MDL_ReadReg(unsigned char reg)
{
    unsigned char value;
    // SPI通信中，读操作需要将寄存器地址最高位置1
    SPI_SelectSlave(lis3mdl_spi_id);
    SPI_TransferByte(reg | LIS3MDL_READ);
    value = SPI_TransferByte(0xFF); // 发送dummy字节，读取返回数据
    SPI_ReleaseSlave(lis3mdl_spi_id);
    return value;
}

// 写入LIS3MDL寄存器
void LIS3MDL_WriteReg(unsigned char reg, unsigned char value)
{
    // SPI通信中，写操作需要将寄存器地址最高位置0
    SPI_SelectSlave(lis3mdl_spi_id);
    SPI_TransferByte(reg & ~LIS3MDL_READ); // 地址最高位清零表示写操作
    SPI_TransferByte(value);
    SPI_ReleaseSlave(lis3mdl_spi_id);
}

// // 连续读取多个LIS3MDL寄存器
// void LIS3MDL_ReadMultiRegisters(unsigned char reg, unsigned char *buffer, unsigned char len)
// {
//     unsigned char i;
//     SPI_SelectSlave(lis3mdl_spi_id);
//     // 发送起始地址，最高位置1表示读操作，增加0x40表示多字节读取
//     SPI_TransferByte(reg | LIS3MDL_READ | 0x40); // 添加0x40位用于自动地址增量
//     // 连续读取多个寄存器数据
//     for(i = 0; i < len; i++)
//     {
//         buffer[i] = SPI_TransferByte(0xFF);
//     }
//     SPI_ReleaseSlave(lis3mdl_spi_id);
// }

void LIS3MDL_ReadMultiRegisters(unsigned char slave_id, unsigned char start_addr,
    unsigned char *buffer, unsigned int count)
{
    SPI_ReadMultiRegisters(slave_id, start_addr | LIS3MDL_READ, buffer, count);
}

// 读取LIS3MDL三轴磁场数据
unsigned char LIS3MDL_ReadData(MagneticData *dataM) //用这个函数来判断spi是否正常
{
    unsigned char buffer[6];
    // 空指针检查
    if (dataM == NULL)
        return 0; // 返回0表示读取失败
        
    // 检查数据是否准备好
    if (!(LIS3MDL_ReadReg(LIS3MDL_STATUS_REG) & 0x08))
        return 2; // 数据未准备好，返回0表示读取失败

    // 使用连续读取函数读取所有数据
    LIS3MDL_ReadMultiRegisters(lis3mdl_spi_id ,LIS3MDL_OUT_X_L, buffer, 6);
    dataM->x_mag = (buffer[1] << 8) | buffer[0];
    dataM->y_mag = (buffer[3] << 8) | buffer[2];
    dataM->z_mag = (buffer[5] << 8) | buffer[4];

    // 计算实际磁场值
    LIS3MDL_CalcMagneticField(dataM, current_scale);
    return 1; // 返回1表示成功读取
}

void LIS3MDL_Updater()
{
    // // 读取磁力计数据
    // if (LIS3MDL_ReadData(&mag_data))
    // {
    //     // 如果正在校准
    //     if (calibration_in_progress)
    //     {
    //         // 添加样本到校准集
    //         LIS3MDL_AddCalibrationSample(&mag_data);
    //     }
    //     else
    //     {
    //         // 校准已完成，计算方位角
    //         LIS3MDL_CalculateHeading(&mag_data);
    //         // 使用计算出的方位角进行其他操作...
    //     }
    // }

    LIS3MDL_ReadData(&mag_data); // 读取磁力计数据
    mag_data.x_gauss_kalman = Mag_Kalman_Update(&mag_x_filter, mag_data.x_mag); // 更新X轴滤波器
    mag_data.y_gauss_kalman = Mag_Kalman_Update(&mag_y_filter, mag_data.y_mag); // 更新Y轴滤波器
    mag_data.z_gauss_kalman = Mag_Kalman_Update(&mag_z_filter, mag_data.z_mag); // 更新Z轴滤波器

    #pragma region 计算航向角
    // 计算航向角

    #pragma endregion
}

// 计算实际磁场值（高斯）
void LIS3MDL_CalcMagneticField(MagneticData *dataM, unsigned char scale)
{
    float sensitivity;

// 空指针检查
    if (dataM == NULL)
        return;

    // 根据量程选择灵敏度系数(LSB/高斯)
    switch (scale)
    {
    case LIS3MDL_FS_4GAUSS:
        sensitivity = 6842.0f; // 6842 LSB/高斯
        break;
    case LIS3MDL_FS_8GAUSS:
        sensitivity = 3421.0f; // 3421 LSB/高斯
        break;
    case LIS3MDL_FS_12GAUSS:
        sensitivity = 2281.0f; // 2281 LSB/高斯
        break;
    case LIS3MDL_FS_16GAUSS:
        sensitivity = 1711.0f; // 1711 LSB/高斯
        break;
    default:
        // 添加非法量程处理
        sensitivity = 6842.0f; // 默认使用±4高斯量程
                               // 这里可以设置一个错误标志或通过其他方式指示错误
    }

    // 将原始数据转换为高斯
    dataM->x_gauss = (float)dataM->x_mag / sensitivity;
    dataM->y_gauss = (float)dataM->y_mag / sensitivity;
    dataM->z_gauss = (float)dataM->z_mag / sensitivity;
}

// 应用校准参数到磁力计数据
void LIS3MDL_ApplyCalibration(MagneticData *dataM)
{
    // 应用硬铁校正（偏移量）
    float x_calibrated ,y_calibrated, z_calibrated;

    x_calibrated = dataM->x_gauss - mag_calibration.x_offset;
    y_calibrated = dataM->y_gauss - mag_calibration.y_offset;
    z_calibrated = dataM->z_gauss - mag_calibration.z_offset;

    // 应用软铁校正（比例因子）
    dataM->x_adj = x_calibrated * mag_calibration.x_scale;
    dataM->y_adj = y_calibrated * mag_calibration.y_scale;
    dataM->z_adj = z_calibrated * mag_calibration.z_scale;
}

float LIS3MDL_CalculateHeading(MagneticData *dataM)
{
    float curAngle;

    // 应用校准参数
    //TODO: 这里需要判断是否已经校准
    //FIXME 但是我认为直接使用校准参数就可
    if (mag_calibration.is_calibrated)
    {
        LIS3MDL_ApplyCalibration(dataM);
    }
    else
    {
        // 未校准！
        return 0.0f;
    }

    // 使用校正后的数据计算方位角
    curAngle = atan2(dataM->y_adj, dataM->x_adj);

    // 将角度范围调整为[0, 2π]
    if (curAngle < 0)
    {
        curAngle += 2 * PI;
    }

    // // 转换为角度
    // heading = heading * 180.0f / PI;
    dataM->heading = curAngle;
    return curAngle;
}


#pragma region 初始化磁场计数据
// 设置校准参数的函数（可以在校准过程完成后调用）
void LIS3MDL_SetCalibrationParams(float x_off, float y_off, float z_off,
                                  float x_scl, float y_scl, float z_scl)
{
    mag_calibration.x_offset = x_off;
    mag_calibration.y_offset = y_off;
    mag_calibration.z_offset = z_off;
    mag_calibration.x_scale = x_scl;
    mag_calibration.y_scale = y_scl;
    mag_calibration.z_scale = z_scl;
    mag_calibration.is_calibrated = 1; // 标记为已校准
}

// 开始磁力计校准过程
unsigned char LIS3MDL_StartCalibration(void)
{
    if (calibration_in_progress)
        return 0; // 已经在校准中

    // 重置校准数据
    memset(mag_samples, 0, sizeof(mag_samples));
    sample_count = 0;
    calibration_in_progress = 1;

    return 1;
}

// 计算校准参数 这个是基础的校准算法
// 这个算法假设磁场是线性的，适用于简单的校准场景qwq
unsigned char LIS3MDL_CalculateCalibration(void)
{
    float x_max, x_min, y_max, y_min, z_max, z_min;
    float x_offset, y_offset, z_offset;
    float x_scale, y_scale, z_scale;
    float avg_scale;
    unsigned int i;

    if (!calibration_in_progress || sample_count < 10) // 至少需要10个样本
        return 0;

    // 初始化最大最小值
    x_max = y_max = z_max = -1000.0f;
    x_min = y_min = z_min = 1000.0f;

    // 找出每个轴的最大和最小值
    for (i = 0; i < sample_count; i++)
    {
        // X轴
        if (mag_samples[i].x_gauss > x_max)
            x_max = mag_samples[i].x_gauss;
        if (mag_samples[i].x_gauss < x_min)
            x_min = mag_samples[i].x_gauss;

        // Y轴
        if (mag_samples[i].y_gauss > y_max)
            y_max = mag_samples[i].y_gauss;
        if (mag_samples[i].y_gauss < y_min)
            y_min = mag_samples[i].y_gauss;

        // Z轴
        if (mag_samples[i].z_gauss > z_max)
            z_max = mag_samples[i].z_gauss;
        if (mag_samples[i].z_gauss < z_min)
            z_min = mag_samples[i].z_gauss;
    }

    // 计算偏移（硬铁校正）
    x_offset = (x_max + x_min) / 2.0f;
    y_offset = (y_max + y_min) / 2.0f;
    z_offset = (z_max + z_min) / 2.0f;

    // 计算比例因子（软铁校正）
    x_scale = (x_max - x_min) / 2.0f;
    y_scale = (y_max - y_min) / 2.0f;
    z_scale = (z_max - z_min) / 2.0f;

    // 计算平均比例因子
    avg_scale = (x_scale + y_scale + z_scale) / 3.0f;

    // 标准化比例因子
    x_scale = avg_scale / x_scale;
    y_scale = avg_scale / y_scale;
    z_scale = avg_scale / z_scale;

    // 设置校准参数
    LIS3MDL_SetCalibrationParams(x_offset, y_offset, z_offset, x_scale, y_scale, z_scale);

    // 结束校准过程
    calibration_in_progress = 0;

    return 1;
}

// 添加一个采样点到校准数据集
unsigned char LIS3MDL_AddCalibrationSample(MagneticData *dataM)
{
float sensitivity;
    
    if (!calibration_in_progress || sample_count >= MAG_CALIB_SAMPLES)
        return 0;

    // 保存原始数据（未校准的）
    mag_samples[sample_count].x_mag = dataM->x_mag;
    mag_samples[sample_count].y_mag = dataM->y_mag;
    mag_samples[sample_count].z_mag = dataM->z_mag;

// 根据当前量程选择正确的灵敏度系数
    switch (current_scale)
    {
    case LIS3MDL_FS_4GAUSS:
        sensitivity = 6842.0f;
        break;
    case LIS3MDL_FS_8GAUSS:
        sensitivity = 3421.0f;
        break;
    case LIS3MDL_FS_12GAUSS:
        sensitivity = 2281.0f;
        break;
    case LIS3MDL_FS_16GAUSS:
        sensitivity = 1711.0f;
        break;
    default:
        sensitivity = 6842.0f; // 默认值
    }

    // 计算高斯值，使用正确的当前灵敏度
    mag_samples[sample_count].x_gauss = (float)dataM->x_mag / sensitivity;
    mag_samples[sample_count].y_gauss = (float)dataM->y_mag / sensitivity;
    mag_samples[sample_count].z_gauss = (float)dataM->z_mag / sensitivity;

    sample_count++;

    // 如果采样数达到预设值，自动计算校准参数
    if (sample_count >= MAG_CALIB_SAMPLES)
        return LIS3MDL_CalculateCalibration();

    return 1;
}

// TODO 请验证算法是否正确和修复错误
//  高级椭球拟合校准算法 <- 适用于更复杂的校准场景(就用这个吧)
//  我认为这个算法只需要在打比赛时候提前校准一次就可以了
unsigned char LIS3MDL_AdvancedCalibration(void)
{
    // 计算椭球拟合参数
    float x_sum = 0, y_sum = 0, z_sum = 0;
    float xx_sum = 0, yy_sum = 0, zz_sum = 0;
    float xy_sum = 0, xz_sum = 0, yz_sum = 0;
    float x, y, z;
    unsigned int i;
    float x_offset, y_offset, z_offset;
    float x_scale, y_scale, z_scale;
    float avg_radius, radius;
    // 应用硬铁校正并计算各轴的半径
    float radius_sum = 0;
    float x_radius_sum = 0, y_radius_sum = 0, z_radius_sum = 0;
    float x_avg_radius, y_avg_radius, z_avg_radius;

    if (!calibration_in_progress || sample_count < 30) // 至少需要30个样本进行椭球拟合 主要还是看效果
        return 0;

    // 计算各种累加和
    for (i = 0; i < sample_count; i++)
    {
        x = mag_samples[i].x_gauss;
        y = mag_samples[i].y_gauss;
        z = mag_samples[i].z_gauss;

        x_sum += x;
        y_sum += y;
        z_sum += z;

        xx_sum += x * x;
        yy_sum += y * y;
        zz_sum += z * z;

        xy_sum += x * y;
        xz_sum += x * z;
        yz_sum += y * z;
    }

    // 计算均值
    x_sum /= sample_count;
    y_sum /= sample_count;
    z_sum /= sample_count;

    //? 简化的椭球拟合算法
    //? 假设椭球的主轴与坐标轴对齐，即椭球方程为：
    //* Ax^2 + By^2 + Cz^2 + Dx + Ey + Fz + G = 0

    // 计算偏移（硬铁校正）- 使用样本均值作为中心点估计
    x_offset = x_sum;
    y_offset = y_sum;
    z_offset = z_sum;

    for (i = 0; i < sample_count; i++) // 计算半径
    {
        x = mag_samples[i].x_gauss - x_offset;
        y = mag_samples[i].y_gauss - y_offset;
        z = mag_samples[i].z_gauss - z_offset;

        radius = sqrt(x * x + y * y + z * z);
        radius_sum += radius;

        x_radius_sum += fabs(x);
        y_radius_sum += fabs(y);
        z_radius_sum += fabs(z);
    }

    // 计算平均半径和各轴的平均投影
    avg_radius = radius_sum / sample_count;
    x_avg_radius = x_radius_sum / sample_count;
    y_avg_radius = y_radius_sum / sample_count;
    z_avg_radius = z_radius_sum / sample_count;

    // 计算缩放因子（软铁校正）
    x_scale = avg_radius / (x_avg_radius * 3.0f);
    y_scale = avg_radius / (y_avg_radius * 3.0f);
    z_scale = avg_radius / (z_avg_radius * 3.0f);

    // 设置校准参数
    LIS3MDL_SetCalibrationParams(x_offset, y_offset, z_offset, x_scale, y_scale, z_scale);

    // 结束校准过程
    calibration_in_progress = 0;

    return 1;
}
#pragma endregion
    // // 读取磁力计数据
    // if(LIS3MDL_ReadData(&mag_data))
    // {
    //     // 如果正在校准
    //     if(calib_state == 1)
    //     {
    //         // 添加样本到校准集
    //         if(!LIS3MDL_AddCalibrationSample(&mag_data))
    //         {
    //             // 样本收集完成，执行高级校准
    //             if(LIS3MDL_AdvancedCalibration())
    //             {
    //                 calib_state = 2;  // 标记校准完成
    //                 // 这里可以添加校准完成的提示或动作
    //             }
    //         }
    //     }
    //     else if(calib_state == 2)
    //     {
    //         // 校准已完成，计算方位角
    //         float heading = LIS3MDL_CalculateHeading(&mag_data);
    //         // 使用计算出的方位角进行其他操作...
    //     }
    // }

    // if(/* 检测到校准按钮被按下 */)
    // {
    //     if(LIS3MDL_StartCalibration())
    //     {
    //         calib_state = 1;  // 标记开始校准
    //         // 这里可以添加校准开始的提示或动作
    //     }
    // }