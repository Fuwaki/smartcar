#ifndef __GYROSCOPE_H__
#define __GYROSCOPE_H__
    #include <AI8051U.H>
    // ICM42688-P SPI从设备地址
    // #define ICM42688_SLAVE_ID           0x01 // 根据SPI设备配置调整

    // ICM42688-P 寄存器地址定义
    #define ICM42688_WHO_AM_I           0x75
    #define ICM42688_WHO_AM_I_VALUE     0x47    // 设备ID，用于验证通信

    // 设备配置寄存器
    #define ICM42688_DEVICE_CONFIG      0x11
    #define ICM42688_DRIVE_CONFIG       0x13
    #define ICM42688_INT_CONFIG         0x14
    #define ICM42688_FIFO_CONFIG        0x16
    #define ICM42688_TEMP_DATA1         0x1D    // 温度数据高字节
    #define ICM42688_TEMP_DATA0         0x1E    // 温度数据低字节

    // 陀螺仪和加速度计数据寄存器
    #define ICM42688_ACCEL_DATA_X1      0x1F    // 加速度X轴高字节
    #define ICM42688_ACCEL_DATA_X0      0x20    // 加速度X轴低字节
    #define ICM42688_ACCEL_DATA_Y1      0x21    // 加速度Y轴高字节
    #define ICM42688_ACCEL_DATA_Y0      0x22    // 加速度Y轴低字节
    #define ICM42688_ACCEL_DATA_Z1      0x23    // 加速度Z轴高字节
    #define ICM42688_ACCEL_DATA_Z0      0x24    // 加速度Z轴低字节

    #define ICM42688_GYRO_DATA_X1       0x25    // 陀螺仪X轴高字节
    #define ICM42688_GYRO_DATA_X0       0x26    // 陀螺仪X轴低字节
    #define ICM42688_GYRO_DATA_Y1       0x27    // 陀螺仪Y轴高字节
    #define ICM42688_GYRO_DATA_Y0       0x28    // 陀螺仪Y轴低字节
    #define ICM42688_GYRO_DATA_Z1       0x29    // 陀螺仪Z轴高字节
    #define ICM42688_GYRO_DATA_Z0       0x2A    // 陀螺仪Z轴低字节

    // 电源管理寄存器
    #define ICM42688_PWR_MGMT0          0x4E
    #define ICM42688_GYRO_CONFIG0       0x4F
    #define ICM42688_ACCEL_CONFIG0      0x50
    #define ICM42688_GYRO_CONFIG1       0x51
    #define ICM42688_GYRO_ACCEL_CONFIG0 0x52
    #define ICM42688_ACCEL_CONFIG1      0x53

    // 中断寄存器
    #define ICM42688_INT_STATUS         0x19
    #define ICM42688_INT_STATUS2        0x1A
    #define ICM42688_INT_STATUS3        0x1B

    // SPI读写标志位
    #define ICM42688_READ_FLAG          0x80  // SPI读标志位，MSB=1表示读操作

    // 功率模式
    #define ICM42688_PWR_MGMT0_TEMP_DISABLE     0x20
    #define ICM42688_PWR_MGMT0_ACCEL_MODE_LN    0x0C // 加速度计低噪声模式
    #define ICM42688_PWR_MGMT0_GYRO_MODE_LN     0x03 // 陀螺仪低噪声模式

    // 陀螺仪满量程范围
    typedef enum {
        GYRO_RANGE_2000_DPS = 0,     // ±2000度/秒
        GYRO_RANGE_1000_DPS = 1,     // ±1000度/秒
        GYRO_RANGE_500_DPS = 2,      // ±500度/秒
        GYRO_RANGE_250_DPS = 3,      // ±250度/秒
        GYRO_RANGE_125_DPS = 4,      // ±125度/秒
        GYRO_RANGE_62_5_DPS = 5,     // ±62.5度/秒
        GYRO_RANGE_31_25_DPS = 6,    // ±31.25度/秒
        GYRO_RANGE_15_625_DPS = 7    // ±15.625度/秒
    } gyro_range_t;

    // 加速度计满量程范围
    typedef enum {
        ACCEL_RANGE_16G = 0,         // ±16g
        ACCEL_RANGE_8G = 1,          // ±8g
        ACCEL_RANGE_4G = 2,          // ±4g
        ACCEL_RANGE_2G = 3           // ±2g
    } accel_range_t;

    // 传感器数据结构
    typedef struct {
        int accel_x;
        int accel_y;
        int accel_z;
        int gyro_x;
        int gyro_y;
        int gyro_z;
        int temp;

        float accel_x_g;
        float accel_y_g;
        float accel_z_g;
        float gyro_x_dps;
        float gyro_y_dps;
        float gyro_z_dps;
        float gyro_x_dps_kf; // 卡尔曼滤波后的陀螺仪X轴数据
        float gyro_y_dps_kf; // 卡尔曼滤波后的陀螺仪Y轴数据
        float gyro_z_dps_kf; // 卡尔曼滤波后的陀螺仪Z轴数据
        float temp_c;

        float true_yaw_angle; //! 真航向角
    } icm42688_data_t;

    extern icm42688_data_t gyro_data; // 声明传感器数据结构体
    extern bit allowUpdate; // 允许更新航向角数据的标志位

    // 初始化ICM42688-P传感器
    // 返回：0-成功，1-失败
    unsigned char ICM42688_Init();

    // 设置陀螺仪量程
    void ICM42688_SetGyroRange(gyro_range_t range);

    // 设置加速度计量程
    void ICM42688_SetAccelRange(accel_range_t range);

    // 读取传感器原始数据
    void ICM42688_ReadSensorData(icm42688_data_t *dataf);

    // 软件复位ICM42688-P
    void ICM42688_Reset();

    // 检查传感器通信是否正常
    unsigned char ICM42688_TestConnection();

    // 获取转换后的温度值（摄氏度）
    float ICM42688_GetTemperature(int raw_temp);

    // 根据当前设置的范围转换陀螺仪原始数据为度/秒
    float ICM42688_GyroConvert(int raw_gyro, gyro_range_t range);

    // 根据当前设置的范围转换加速度计原始数据为G
    float ICM42688_AccelConvert(int raw_accel, accel_range_t range);
    
    void Gyro_Updater();

    // 卡尔曼滤波器结构体定义
    typedef struct {
        float x;      // 状态估计
        float P;      // 估计误差协方差
        float Q;      // 过程噪声协方差
        float R;      // 测量噪声协方差
        float K;      // 卡尔曼增益
    } kalman_filter_t;

    // 卡尔曼滤波函数声明
    void kalman_init(kalman_filter_t *filter, float Q, float R, float P_initial, float x_initial);
    float kalman_update(kalman_filter_t *filter, float measurement);
    void apply_kalman_filter(icm42688_data_t *raw_data, icm42688_data_t *filtered_data, 
                            kalman_filter_t *kf_accel_x, kalman_filter_t *kf_accel_y, kalman_filter_t *kf_accel_z,
                            kalman_filter_t *kf_gyro_x, kalman_filter_t *kf_gyro_y, kalman_filter_t *kf_gyro_z);
    void init_gyro_kalman_filters(kalman_filter_t *kf_accel_x, kalman_filter_t *kf_accel_y, kalman_filter_t *kf_accel_z,
                                kalman_filter_t *kf_gyro_x, kalman_filter_t *kf_gyro_y, kalman_filter_t *kf_gyro_z);

    void yaw_angle_init(); // 初始化航向角

#endif // __GYROSCOPE_H__