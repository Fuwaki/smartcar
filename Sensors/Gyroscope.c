#include <AI8051U.H>
#include <intrins.h>
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "Gyroscope.h"


void Delay500ms(void)	//@35.000MHz
{
	unsigned long edata i;

	_nop_();
	_nop_();
	i = 4374998UL;
	while (i) i--;
}

float fast_sqrt(float x) // 速度更快的平方根计算
{
	float halfx = 0.5f * x;
	float y = x;
	long i = *(long*) & y;
    //0x5f3759df是一个平方根倒数速算法
	i = 0x5f3759df - (i >> 1);
	y = *(float*) & i;
	y = y * (1.5f - (halfx * y * y));
	return y;
}

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
 * @brief 对陀螺仪和加速度数据应用卡尔曼滤波
 * @param raw_data 原始传感器数据
 * @param filtered_data 滤波后的数据
 * @param kf_accel_x,kf_accel_y,kf_accel_z 加速度数据的滤波器
 * @param kf_gyro_x,kf_gyro_y,kf_gyro_z 陀螺仪数据的滤波器
 */
void apply_kalman_filter(icm426888_data_t *raw_data, icm426888_data_t *filtered_data, 
                        kalman_filter_t *kf_accel_x, kalman_filter_t *kf_accel_y, kalman_filter_t *kf_accel_z,
                        kalman_filter_t *kf_gyro_x, kalman_filter_t *kf_gyro_y, kalman_filter_t *kf_gyro_z) 
{
    // 更新加速度数据
    filtered_data->accel_x = kalman_update(kf_accel_x, raw_data->accel_x);
    filtered_data->accel_y = kalman_update(kf_accel_y, raw_data->accel_y);
    filtered_data->accel_z = kalman_update(kf_accel_z, raw_data->accel_z);
    
    // 更新陀螺仪数据
    filtered_data->gyro_x = kalman_update(kf_gyro_x, raw_data->gyro_x);
    filtered_data->gyro_y = kalman_update(kf_gyro_y, raw_data->gyro_y);
    filtered_data->gyro_z = kalman_update(kf_gyro_z, raw_data->gyro_z);
    
    // 温度数据通常不需要滤波，直接传递
    filtered_data->temperature = raw_data->temperature;
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
 * @brief Parse ICM-426888 data from SIP packet
 * @param sip_payload Pointer to SIP payload data
 * @param payload_len Length of the SIP payload
 * @param sensor_data Pointer to store the parsed sensor data
 * @return 0 if successful, negative error code otherwise
 */
int icm426888_parse_sip_data(const char *sip_payload, unsigned int payload_len, icm426888_data_t *sensor_data)
{
    int raw_accel_x, raw_accel_y, raw_accel_z;
    int raw_gyro_x, raw_gyro_y, raw_gyro_z;
    char raw_temp;
    float accel_scale, gyro_scale; 
    
    if (sip_payload == NULL || sensor_data == NULL || payload_len < 16) {
        return -1; // Invalid parameters
    }
    
    if (sip_payload[0] != 0x49 || sip_payload[1] != 0x43 || sip_payload[2] != 0x4D) {
        return -2; // Invalid header - should start with "ICM"
    }

    // 使用乘法代替位移
    raw_accel_x = ((unsigned char)sip_payload[3] * 256) + (unsigned char)sip_payload[4];
    raw_accel_y = ((unsigned char)sip_payload[5] * 256) + (unsigned char)sip_payload[6];
    raw_accel_z = ((unsigned char)sip_payload[7] * 256) + (unsigned char)sip_payload[8];
    
    raw_gyro_x = ((unsigned char)sip_payload[9] * 256) + (unsigned char)sip_payload[10];
    raw_gyro_y = ((unsigned char)sip_payload[11] * 256) + (unsigned char)sip_payload[12];
    raw_gyro_z = ((unsigned char)sip_payload[13] * 256) + (unsigned char)sip_payload[14];
    
    raw_temp = sip_payload[15];
    
    accel_scale = 16.0f / 32768.0f * 1000.0f; // 转换为 mg
    gyro_scale = 2000.0f / 32768.0f;          // 转换为 degrees/s
    
    sensor_data->accel_x = raw_accel_x * accel_scale;
    sensor_data->accel_y = raw_accel_y * accel_scale;
    sensor_data->accel_z = raw_accel_z * accel_scale;
    
    sensor_data->gyro_x = raw_gyro_x * gyro_scale;
    sensor_data->gyro_y = raw_gyro_y * gyro_scale;
    sensor_data->gyro_z = raw_gyro_z * gyro_scale;
    
    sensor_data->temperature = (float)raw_temp + 25.0f;
    
    return 0; // Success
}

// Ciallo～(∠�9�9ω< )⌒★     
// Ciallo～(∠�9�9ω< )⌒★     
// Ciallo～(∠�9�9ω< )⌒★     


// void test_icm426888_parser(void)
// {
//     // 模拟的SIP数据包（实际应用中，这些数据将从通信接口获取）
//     // 格式: [ICM标识符(3)][accel_x(2)][accel_y(2)][accel_z(2)][gyro_x(2)][gyro_y(2)][gyro_z(2)][temp(1)]
//     unsigned char test_sip_data[16] = {
//         0x49, 0x43, 0x4D,             // "ICM" 标识符
//         0x01, 0x23,                   // accel_x raw data (291)
//         0x45, 0x67,                   // accel_y raw data (17767)
//         0x89, 0xAB,                   // accel_z raw data (-30293)
//         0xCD, 0xEF,                   // gyro_x raw data (-12817)
//         0x02, 0x46,                   // gyro_y raw data (582)
//         0x8A, 0xBC,                   // gyro_z raw data (-30020)
//         0x15                         // temp raw data (21°C)
//     };
    
//     // 声明传感器数据结构体
//     icm426888_data_t sensor_data;
//     int result;
    
//     // 调用解析函数
//     result = icm426888_parse_sip_data((char*)test_sip_data, 16, &sensor_data);
    
//     // 检查结果
//     if (result == 0) {
//         // 打印解析的数据（实际应用中可以进行其他处理）
//         printf("加速度数据 (mg):\n");
//         printf("  X: %.2f\n", sensor_data.accel_x);
//         printf("  Y: %.2f\n", sensor_data.accel_y);
//         printf("  Z: %.2f\n", sensor_data.accel_z);
        
//         printf("陀螺仪数据 (degrees/s):\n");
//         printf("  X: %.2f\n", sensor_data.gyro_x);
//         printf("  Y: %.2f\n", sensor_data.gyro_y);
//         printf("  Z: %.2f\n", sensor_data.gyro_z);
        
//         printf("温度: %.1f°C\n", sensor_data.temperature);
        
//         // 在实际应用中，您可能会使用这些数据来控制设备或进行进一步计算
//         // 例如，计算方位、检测运动等
//     }
//     else {
//         printf("解析SIP数据失败，错误代码: %d\n", result);
//     }
// }

// /**
//  * @brief 实际应用示例：从UART接收SIP数据并解析
//  * 此函数假设您的系统有UART接收功能
//  */
// void process_uart_gyro_data(void)
// {
//     unsigned char sip_buffer[32]; // 缓冲区，根据需要调整大小
//     unsigned int received_bytes = 0;
//     icm426888_data_t sensor_data;
    
//     // 这里应该是您的接收UART数据的代码
//     // 例如：received_bytes = uart_receive_data(sip_buffer, 32);
    
//     if (received_bytes >= 16) { // 确保接收到足够的数据
//         if (icm426888_parse_sip_data((char*)sip_buffer, received_bytes, &sensor_data) == 0) {
//             // 使用解析后的传感器数据
//             // 例如：计算设备倾角
//             float pitch = fast_sqrt(sensor_data.accel_x * sensor_data.accel_x + 
//                                     sensor_data.accel_z * sensor_data.accel_z);
//             // 用于运动控制、状态监测等
//         }
//     }
// }

// int main()
// {
//     // 测试解析函数
//     test_icm426888_parser();
    
//     // 实际应用示例：处理UART接收的SIP数据
//     process_uart_gyro_data();
    
//     return 0;
// }