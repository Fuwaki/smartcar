#ifndef __GYROSCOPE_H__
#define __GYROSCOPE_H__

    typedef struct {
        // Accelerometer data (mg)
        float accel_x;
        float accel_y;
        float accel_z;
        // Gyroscope data (degrees/s)
        float gyro_x;
        float gyro_y;
        float gyro_z;
        // Temperature data (Celsius)
        float temperature;
    } icm426888_data_t;

    /**
     * @brief Parse ICM-426888 data from SIP packet
     * @param sip_payload Pointer to SIP payload data
     * @param payload_len Length of the SIP payload
     * @param sensor_data Pointer to store the parsed sensor data
     * @return 0 if successful, negative error code otherwise
     */
    int icm426888_parse_sip_data(const char *sip_payload, unsigned int payload_len, icm426888_data_t *sensor_data);

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
    void apply_kalman_filter(icm426888_data_t *raw_data, icm426888_data_t *filtered_data, 
                            kalman_filter_t *kf_accel_x, kalman_filter_t *kf_accel_y, kalman_filter_t *kf_accel_z,
                            kalman_filter_t *kf_gyro_x, kalman_filter_t *kf_gyro_y, kalman_filter_t *kf_gyro_z);
    void init_gyro_kalman_filters(kalman_filter_t *kf_accel_x, kalman_filter_t *kf_accel_y, kalman_filter_t *kf_accel_z,
                                kalman_filter_t *kf_gyro_x, kalman_filter_t *kf_gyro_y, kalman_filter_t *kf_gyro_z);

#endif