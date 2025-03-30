#include <AI8051U.H>
#include <intrins.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "Magnetic.h"
#include "SPI_MultiDevice.h"

#define PI 3.14159265358979323846 // 这么长怎么你了!
// 当前选定的量程
static unsigned char current_scale = LIS3MDL_FS_4GAUSS;
// LIS3MDL的SPI设备ID
static unsigned char lis3mdl_spi_id = 0xFF; //初始值为0xFF表示未注册

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
static MagneticData mag_samples[MAG_CALIB_SAMPLES];
static unsigned int sample_count = 0;
static unsigned char calibration_in_progress = 0;

// SPI通信地址位定义
#define LIS3MDL_READ 0x80  // 读操作最高位为1
#define LIS3MDL_WRITE 0x00 // 写操作最高位为0

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
void LIS3MDL_Init(void)
{
    unsigned char device_id;
    // 配置SPI从设备
    spi_slave_config_t lis3mdl_config;
    lis3mdl_config.cs_port = 1;
    lis3mdl_config.cs_pin = 6;
    lis3mdl_config.mode = SPI_MODE3; // LIS3MDL使用SPI模式3 (CPOL=1, CPHA=1)
    lis3mdl_config.clock_div = 0x00; // SPI时钟速率设置，根据需要调整

    // 注册LIS3MDL为SPI从设备
    lis3mdl_spi_id = SPI_RegisterSlave(&lis3mdl_config);

    if (lis3mdl_spi_id == 0xFF)
    {
        // 注册失败处理
        return;
    }

    // 1. 复位设备
    LIS3MDL_WriteReg(LIS3MDL_CTRL_REG2, 0x0C); // 软复位

    Mag_Delay(); // 等待复位完成

    // 验证设备ID
    device_id = LIS3MDL_ReadReg(LIS3MDL_WHO_AM_I);
    if (device_id != 0x3D)
    { // LIS3MDL的WHO_AM_I寄存器值应为0x3D
        // 设备ID错误，可以在此添加错误处理代码
        return;
    }

    // 2. 配置传感器
    // CTRL_REG1: 温度传感器使能, XY轴高性能模式, 80Hz输出速率
    LIS3MDL_WriteReg(LIS3MDL_CTRL_REG1, 0xFC);

    // CTRL_REG2: 设置量程为±4高斯
    LIS3MDL_WriteReg(LIS3MDL_CTRL_REG2, LIS3MDL_FS_4GAUSS);
    current_scale = LIS3MDL_FS_4GAUSS;

    // CTRL_REG3: 连续转换模式
    LIS3MDL_WriteReg(LIS3MDL_CTRL_REG3, 0x00);

    // CTRL_REG4: Z轴高性能模式, 大端数据
    LIS3MDL_WriteReg(LIS3MDL_CTRL_REG4, 0x0C);

    // CTRL_REG5: 快速读取模式
    LIS3MDL_WriteReg(LIS3MDL_CTRL_REG5, 0x40);

    // 初始化校准参数
    LIS3MDL_InitCalibration();
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

// 读取LIS3MDL三轴磁场数据
// 返回值: 1-成功读取数据, 0-数据未就绪
unsigned char LIS3MDL_ReadData(MagneticData *dataM)
{
    unsigned char x_l, x_h, y_l, y_h, z_l, z_h;

    // 检查数据是否准备好
    if (!(LIS3MDL_ReadReg(LIS3MDL_STATUS_REG) & 0x08))
        return 0; // 数据未准备好，返回0表示读取失败

    // 在SPI模式下，可以使用连续读取模式提高效率
    SPI_SelectSlave(lis3mdl_spi_id);
    SPI_TransferByte(LIS3MDL_OUT_X_L | LIS3MDL_READ);
    x_l = SPI_TransferByte(0xFF); // 发送虚拟数据，读取返回值方便后续读取
    x_h = SPI_TransferByte(0xFF);
    y_l = SPI_TransferByte(0xFF);
    y_h = SPI_TransferByte(0xFF);
    z_l = SPI_TransferByte(0xFF);
    z_h = SPI_TransferByte(0xFF);
    SPI_ReleaseSlave(lis3mdl_spi_id);

    // 合并数据
    dataM->x = (int)((x_h << 8) | x_l);
    dataM->y = (int)((y_h << 8) | y_l);
    dataM->z = (int)((z_h << 8) | z_l);

    // 计算实际磁场值
    LIS3MDL_CalcMagneticField(dataM, current_scale);

    return 1; // 返回1表示成功读取
}

// 计算实际磁场值（高斯）
void LIS3MDL_CalcMagneticField(MagneticData *dataM, unsigned char scale)
{
    float sensitivity;

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
    dataM->x_gauss = (float)dataM->x / sensitivity;
    dataM->y_gauss = (float)dataM->y / sensitivity;
    dataM->z_gauss = (float)dataM->z / sensitivity;
}

// 应用校准参数到磁力计数据
void LIS3MDL_ApplyCalibration(MagneticData *dataM)
{
    // 应用硬铁校正（偏移量）
    float x_calibrated = dataM->x_gauss - mag_calibration.x_offset;
    float y_calibrated = dataM->y_gauss - mag_calibration.y_offset;
    float z_calibrated = dataM->z_gauss - mag_calibration.z_offset;

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
    curAngle = atan2(dataM->y_gauss, dataM->x_gauss);

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
    if (!calibration_in_progress || sample_count >= MAG_CALIB_SAMPLES)
        return 0;

    // 保存原始数据（未校准的）
    mag_samples[sample_count].x = dataM->x;
    mag_samples[sample_count].y = dataM->y;
    mag_samples[sample_count].z = dataM->z;

    // 计算高斯值
    mag_samples[sample_count].x_gauss = (float)dataM->x / 6842.0f; // 假设使用±4高斯量程
    mag_samples[sample_count].y_gauss = (float)dataM->y / 6842.0f;
    mag_samples[sample_count].z_gauss = (float)dataM->z / 6842.0f;

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

    // 简化的椭球拟合算法
    // 假设椭球的主轴与坐标轴对齐，即椭球方程为：
    // Ax^2 + By^2 + Cz^2 + Dx + Ey + Fz + G = 0

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