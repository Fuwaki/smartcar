#ifndef __MAGNETIC_H__
#define __MAGNETIC_H__

    // LIS3MDL I2C地址
    #define LIS3MDL_ADDR        0x1E    // 或0x1C，取决于SA1引脚(高电平为0x1E)

    // LIS3MDL寄存器地址
    #define LIS3MDL_WHO_AM_I    0x0F
    #define LIS3MDL_CTRL_REG1   0x20
    #define LIS3MDL_CTRL_REG2   0x21
    #define LIS3MDL_CTRL_REG3   0x22
    #define LIS3MDL_CTRL_REG4   0x23
    #define LIS3MDL_CTRL_REG5   0x24
    #define LIS3MDL_STATUS_REG  0x27
    #define LIS3MDL_OUT_X_L     0x28
    #define LIS3MDL_OUT_X_H     0x29
    #define LIS3MDL_OUT_Y_L     0x2A
    #define LIS3MDL_OUT_Y_H     0x2B
    #define LIS3MDL_OUT_Z_L     0x2C
    #define LIS3MDL_OUT_Z_H     0x2D
    #define LIS3MDL_TEMP_OUT_L  0x2E
    #define LIS3MDL_TEMP_OUT_H  0x2F

    // 量程设置
    #define LIS3MDL_FS_4GAUSS   0x00
    #define LIS3MDL_FS_8GAUSS   0x20
    #define LIS3MDL_FS_12GAUSS  0x40
    #define LIS3MDL_FS_16GAUSS  0x60

    // 数据存储结构体
    typedef struct {
        int x_mag;
        int y_mag;
        int z_mag;
        float x_gauss;
        float y_gauss;
        float z_gauss;
        float x_adj;
        float y_adj;
        float z_adj;
        float x_gauss_kalman;
        float y_gauss_kalman;
        float z_gauss_kalman;
        float heading;
    } MagneticData;

    extern MagneticData mag_data;  // 磁力计数据

    // 函数声明
    unsigned char LIS3MDL_Init(void);
    unsigned char LIS3MDL_ReadReg(unsigned char reg);
    void LIS3MDL_WriteReg(unsigned char reg, unsigned char value);
    void LIS3MDL_ReadMultiRegisters(unsigned char slave_id, unsigned char start_addr,
        unsigned char *buffer, unsigned int count);
    unsigned char LIS3MDL_ReadData(MagneticData* dataM);  // 修改返回类型为unsigned char
    void LIS3MDL_CalcMagneticField(MagneticData* dataM, unsigned char scale);
    float LIS3MDL_CalculateHeading(MagneticData *dataM);  // 计算方位角函数声明
    void LIS3MDL_ApplyCalibration(MagneticData *dataM);
    void LIS3MDL_InitCalibration(void);
    void LIS3MDL_SetCalibrationParams(float x_off, float y_off, float z_off, 
        float x_scl, float y_scl, float z_scl);
    unsigned char LIS3MDL_AddCalibrationSample(MagneticData *dataM);
    unsigned char LIS3MDL_CalculateCalibration(void);
    unsigned char LIS3MDL_AdvancedCalibration(void);
    void LIS3MDL_Updater(void);
#endif