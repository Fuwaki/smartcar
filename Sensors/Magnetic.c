#include <AI8051U.H>
#include <intrins.h>
#include <string.h>
#include <math.h>
#include "Magnetic.h"
#include "SPI_MultiDevice.h"

// 当前选定的量程
static unsigned char current_scale = LIS3MDL_FS_4GAUSS;

// LIS3MDL的SPI设备ID
static unsigned char lis3mdl_spi_id = 0xFF;

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

// SPI通信地址位定义
#define LIS3MDL_READ 0x80  // 读操作最高位为1
#define LIS3MDL_WRITE 0x00 // 写操作最高位为0

void Delay(void) //@40.000MHz
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
    // 这里可以从EEPROM或Flash中加载校准参数
    // 或者使用默认的校准参数

    // 默认硬铁校正参数（理想情况下应通过校准过程确定）
    mag_calibration.x_offset = 0.0f;
    mag_calibration.y_offset = 0.0f;
    mag_calibration.z_offset = 0.0f;

    // 默认软铁校正参数（理想情况下应通过校准过程确定）
    mag_calibration.x_scale = 1.0f;
    mag_calibration.y_scale = 1.0f;
    mag_calibration.z_scale = 1.0f;

    mag_calibration.is_calibrated = 0; // 标记为未校准

    // 注意：在实际应用中，应该提供一个校准过程来获取这些参数
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

    Delay(); // 等待复位完成

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
    x_l = SPI_TransferByte(0xFF);
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
    dataM->x_gauss = x_calibrated * mag_calibration.x_scale;
    dataM->y_gauss = y_calibrated * mag_calibration.y_scale;
    dataM->z_gauss = z_calibrated * mag_calibration.z_scale;
}

float LIS3MDL_CalculateHeading(MagneticData *dataM)
{
    float heading;

    // 应用校准参数
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
    heading = atan2(dataM->y_gauss, dataM->x_gauss);

    // 将角度范围调整为[0, 2π]
    if (heading < 0)
    {
        heading += 2 * 3.14159265f;
    }

    return heading;
}

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
