#ifndef __GYROSCOPE_H__
#define __GYROSCOPE_H__
    // 卡尔曼滤波器结构体定义
    typedef struct {
        float Q_angle;   // 过程噪声协方差
        float Q_bias;    // 过程噪声协方差
        float R_measure; // 测量噪声协方差
        
        float angle;     // 角度
        float bias;      // 角速度偏差
        
        float P[2][2];   // 误差协方差矩阵
    } GyroKalman_t;

    /**
     * @brief 初始化卡尔曼滤波器
     * @param kalman 卡尔曼滤波器结构体指针
     * @param Q_angle 角度过程噪声协方差
     * @param Q_bias 偏差过程噪声协方差
     * @param R_measure 测量噪声协方差
     */
    void Gyro_KalmanInit(GyroKalman_t *kalman, float Q_angle, float Q_bias, float R_measure);

    /**
     * @brief 卡尔曼滤波处理函数
     * @param kalman 卡尔曼滤波器结构体指针
     * @param newAngle 测量的角度值
     * @param newRate 测量的角速度值
     * @param dt 采样时间间隔(秒)
     * @return 滤波后的角度值
     */
    float Gyro_KalmanFilter(GyroKalman_t *kalman, float newAngle, float newRate, float dt);

    /**
     * @brief 应用卡尔曼滤波处理陀螺仪数据示例
     * @param rawAngle 原始角度数据
     * @param rawRate 原始角速度数据
     * @param filteredAngle 滤波后的角度指针
     */
    void Gyro_ProcessData(float rawAngle, float rawRate, float *filteredAngle);

#endif __GYROSCOPE_H__