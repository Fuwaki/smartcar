#include <AI8051U.H>
#include <intrins.h>
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "Gyroscope.h"

// ICM42688-P�Ĵ�����ַ����
#define ICM42688_WHOAMI           0x75
#define ICM42688_PWR_MGMT0        0x4E
#define ICM42688_GYRO_CONFIG0     0x4F
#define ICM42688_ACCEL_CONFIG0    0x50
#define ICM42688_GYRO_DATA_X1     0x25
#define ICM42688_GYRO_DATA_X0     0x26
#define ICM42688_GYRO_DATA_Y1     0x27
#define ICM42688_GYRO_DATA_Y0     0x28
#define ICM42688_GYRO_DATA_Z1     0x29
#define ICM42688_GYRO_DATA_Z0     0x2A
#define ICM42688_ACCEL_DATA_X1    0x1F
#define ICM42688_ACCEL_DATA_X0    0x20
#define ICM42688_ACCEL_DATA_Y1    0x21
#define ICM42688_ACCEL_DATA_Y0    0x22
#define ICM42688_ACCEL_DATA_Z1    0x23
#define ICM42688_ACCEL_DATA_Z0    0x24


void Delay500ms(void)	//@35.000MHz
{
	unsigned long edata i;

	_nop_();
	_nop_();
	i = 4374998UL;
	while (i) i--;
}

float fast_sqrt(float x) // �ٶȸ����ƽ��������
{
	float halfx = 0.5f * x;
	float y = x;
	long i = *(long*) & y;
    //0x5f3759df��һ��ƽ�����������㷨
	i = 0x5f3759df - (i >> 1);
	y = *(float*) & i;
	y = y * (1.5f - (halfx * y * y));
	return y;
}

/**
 * @brief ��ʼ���������˲���
 * @param filter �˲����ṹ��ָ��
 * @param Q ��������Э����
 * @param R ��������Э����
 * @param P_initial ��ʼ�������Э����
 * @param x_initial ��ʼ״̬����
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
 * @brief �������˲����²���
 * @param filter �˲����ṹ��ָ��
 * @param measurement ��ǰ����ֵ
 * @return �˲���Ĺ���ֵ
 */
float kalman_update(kalman_filter_t *filter, float measurement) 
{
    // Ԥ�ⲽ��
    // x = x (״̬Ԥ�⣬��ģ���±��ֲ���)
    filter->P = filter->P + filter->Q;
    
    // ���²���
    filter->K = filter->P / (filter->P + filter->R);
    filter->x = filter->x + filter->K * (measurement - filter->x);
    filter->P = (1 - filter->K) * filter->P;
    
    return filter->x;
}

/**
 * @brief �������Ǻͼ��ٶ�����Ӧ�ÿ������˲�
 * @param raw_data ԭʼ����������
 * @param filtered_data �˲��������
 * @param kf_accel_x,kf_accel_y,kf_accel_z ���ٶ����ݵ��˲���
 * @param kf_gyro_x,kf_gyro_y,kf_gyro_z ���������ݵ��˲���
 */
void apply_kalman_filter(icm426888_data_t *raw_data, icm426888_data_t *filtered_data, 
                        kalman_filter_t *kf_accel_x, kalman_filter_t *kf_accel_y, kalman_filter_t *kf_accel_z,
                        kalman_filter_t *kf_gyro_x, kalman_filter_t *kf_gyro_y, kalman_filter_t *kf_gyro_z) 
{
    // ���¼��ٶ�����
    filtered_data->accel_x = kalman_update(kf_accel_x, raw_data->accel_x);
    filtered_data->accel_y = kalman_update(kf_accel_y, raw_data->accel_y);
    filtered_data->accel_z = kalman_update(kf_accel_z, raw_data->accel_z);
    
    // ��������������
    filtered_data->gyro_x = kalman_update(kf_gyro_x, raw_data->gyro_x);
    filtered_data->gyro_y = kalman_update(kf_gyro_y, raw_data->gyro_y);
    filtered_data->gyro_z = kalman_update(kf_gyro_z, raw_data->gyro_z);
    
    // �¶�����ͨ������Ҫ�˲���ֱ�Ӵ���
    filtered_data->temperature = raw_data->temperature;
}

/**
 * @brief ��ʼ���������������ݵĿ������˲���
 * @param kf_accel_x,kf_accel_y,kf_accel_z ���ٶ����ݵ��˲���
 * @param kf_gyro_x,kf_gyro_y,kf_gyro_z ���������ݵ��˲���
 */
void init_gyro_kalman_filters(kalman_filter_t *kf_accel_x, kalman_filter_t *kf_accel_y, kalman_filter_t *kf_accel_z,
                            kalman_filter_t *kf_gyro_x, kalman_filter_t *kf_gyro_y, kalman_filter_t *kf_gyro_z)
{
    // ��ʼ�����ٶȼ��˲���
    // �����ɸ���ʵ��Ӧ�õ�����Q(��������), R(��������), ��ʼP, ��ʼx
    kalman_init(kf_accel_x, 0.01f, 0.1f, 1.0f, 0.0f);
    kalman_init(kf_accel_y, 0.01f, 0.1f, 1.0f, 0.0f);
    kalman_init(kf_accel_z, 0.01f, 0.1f, 1.0f, 0.0f);
    
    // ��ʼ���������˲���
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

    // ʹ�ó˷�����λ��ɵ��keil
    raw_accel_x = ((unsigned char)sip_payload[3] * 256) + (unsigned char)sip_payload[4];
    raw_accel_y = ((unsigned char)sip_payload[5] * 256) + (unsigned char)sip_payload[6];
    raw_accel_z = ((unsigned char)sip_payload[7] * 256) + (unsigned char)sip_payload[8];
    
    raw_gyro_x = ((unsigned char)sip_payload[9] * 256) + (unsigned char)sip_payload[10];
    raw_gyro_y = ((unsigned char)sip_payload[11] * 256) + (unsigned char)sip_payload[12];
    raw_gyro_z = ((unsigned char)sip_payload[13] * 256) + (unsigned char)sip_payload[14];
    
    raw_temp = sip_payload[15];
    
    accel_scale = 16.0f / 32768.0f * 1000.0f; // ת��Ϊ mg
    gyro_scale = 2000.0f / 32768.0f;          // ת��Ϊ degrees/s
    
    sensor_data->accel_x = raw_accel_x * accel_scale;
    sensor_data->accel_y = raw_accel_y * accel_scale;
    sensor_data->accel_z = raw_accel_z * accel_scale;
    
    sensor_data->gyro_x = raw_gyro_x * gyro_scale;
    sensor_data->gyro_y = raw_gyro_y * gyro_scale;
    sensor_data->gyro_z = raw_gyro_z * gyro_scale;
    

    sensor_data->temperature = (float)raw_temp + 25.0f;
    
    return 0; // Success
}

// Ciallo��(�ρ9�9��< )�С�     
// Ciallo��(�ρ9�9��< )�С�     
// Ciallo��(�ρ9�9��< )�С�     


// void test_icm426888_parser(void)
// {
//     // ģ���SIP���ݰ���ʵ��Ӧ���У���Щ���ݽ���ͨ�Žӿڻ�ȡ��
//     // ��ʽ: [ICM��ʶ��(3)][accel_x(2)][accel_y(2)][accel_z(2)][gyro_x(2)][gyro_y(2)][gyro_z(2)][temp(1)]
//     unsigned char test_sip_data[16] = {
//         0x49, 0x43, 0x4D,             // "ICM" ��ʶ��
//         0x01, 0x23,                   // accel_x raw data (291)
//         0x45, 0x67,                   // accel_y raw data (17767)
//         0x89, 0xAB,                   // accel_z raw data (-30293)
//         0xCD, 0xEF,                   // gyro_x raw data (-12817)
//         0x02, 0x46,                   // gyro_y raw data (582)
//         0x8A, 0xBC,                   // gyro_z raw data (-30020)
//         0x15                         // temp raw data (21��C)
//     };
    
//     // �������������ݽṹ��
//     icm426888_data_t sensor_data;
//     int result;
    
//     // ���ý�������
//     result = icm426888_parse_sip_data((char*)test_sip_data, 16, &sensor_data);
    
//     // �����
//     if (result == 0) {
//         // ��ӡ���������ݣ�ʵ��Ӧ���п��Խ�����������
//         printf("���ٶ����� (mg):\n");
//         printf("  X: %.2f\n", sensor_data.accel_x);
//         printf("  Y: %.2f\n", sensor_data.accel_y);
//         printf("  Z: %.2f\n", sensor_data.accel_z);
        
//         printf("���������� (degrees/s):\n");
//         printf("  X: %.2f\n", sensor_data.gyro_x);
//         printf("  Y: %.2f\n", sensor_data.gyro_y);
//         printf("  Z: %.2f\n", sensor_data.gyro_z);
        
//         printf("�¶�: %.1f��C\n", sensor_data.temperature);
        
//         // ��ʵ��Ӧ���У������ܻ�ʹ����Щ�����������豸����н�һ������
//         // ���磬���㷽λ������˶���
//     }
//     else {
//         printf("����SIP����ʧ�ܣ��������: %d\n", result);
//     }
// }

// /**
//  * @brief ʵ��Ӧ��ʾ������UART����SIP���ݲ�����
//  * �˺�����������ϵͳ��UART���չ���
//  */
// void process_uart_gyro_data(void)
// {
//     unsigned char sip_buffer[32]; // ��������������Ҫ������С
//     unsigned int received_bytes = 0;
//     icm426888_data_t sensor_data;
    
//     // ����Ӧ�������Ľ���UART���ݵĴ���
//     // ���磺received_bytes = uart_receive_data(sip_buffer, 32);
    
//     if (received_bytes >= 16) { // ȷ�����յ��㹻������
//         if (icm426888_parse_sip_data((char*)sip_buffer, received_bytes, &sensor_data) == 0) {
//             // ʹ�ý�����Ĵ���������
//             // ���磺�����豸���
//             float pitch = fast_sqrt(sensor_data.accel_x * sensor_data.accel_x + 
//                                     sensor_data.accel_z * sensor_data.accel_z);
//             // �����˶����ơ�״̬����
//         }
//     }
// }

// int main()
// {
//     // ���Խ�������
//     test_icm426888_parser();
    
//     // ʵ��Ӧ��ʾ��������UART���յ�SIP����
//     process_uart_gyro_data();
    
//     return 0;
// }