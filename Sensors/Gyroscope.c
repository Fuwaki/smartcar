#include <AI8051U.H>
#include <intrins.h>
#include <math.h>

// ICM42688-P寄存器地址定义
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

// 姿态参数结构体
typedef struct {
    float AngleX; // X轴角度
    float AngleY; // Y轴角度
    float AngleZ; // Z轴角度
} Gyroscope;

// 卡尔曼滤波参数
typedef struct {
    float Q_angle;   // 过程噪声协方差
    float Q_bias;    // 过程噪声协方差
    float R_measure; // 测量噪声协方差
    float angle;     // 角度
    float bias;      // 角速度偏差
    float P[2][2];   // 协方差矩阵
} Kalman_t;

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

// 卡尔曼滤波初始化
void Kalman_Init(Kalman_t *kalman)
{
    kalman->Q_angle = 0.001f;
    kalman->Q_bias = 0.003f;
    kalman->R_measure = 0.03f;
    
    kalman->angle = 0.0f;
    kalman->bias = 0.0f;
    
    kalman->P[0][0] = 0.0f;
    kalman->P[0][1] = 0.0f;
    kalman->P[1][0] = 0.0f;
    kalman->P[1][1] = 0.0f;
}

// 卡尔曼滤波器
float Kalman_Filter(Kalman_t *kalman, float new_angle, float new_rate, float dt)
{
    float rate = new_rate - kalman->bias;
    kalman->angle += dt * rate;
    
    // 更新估计协方差矩阵
    kalman->P[0][0] += dt * (dt * kalman->P[1][1] - kalman->P[0][1] - kalman->P[1][0] + kalman->Q_angle);
    kalman->P[0][1] -= dt * kalman->P[1][1];
    kalman->P[1][0] -= dt * kalman->P[1][1];
    kalman->P[1][1] += kalman->Q_bias * dt;
    
    // 计算卡尔曼增益
    float S = kalman->P[0][0] + kalman->R_measure;
    float K[2];
    K[0] = kalman->P[0][0] / S;
    K[1] = kalman->P[1][0] / S;
    
    // 计算角度误差
    float y = new_angle - kalman->angle;
    
    // 更新滤波值
    kalman->angle += K[0] * y;
    kalman->bias += K[1] * y;
    
    // 更新协方差矩阵
    float P00_temp = kalman->P[0][0];
    float P01_temp = kalman->P[0][1];
    
    kalman->P[0][0] -= K[0] * P00_temp;
    kalman->P[0][1] -= K[0] * P01_temp;
    kalman->P[1][0] -= K[1] * P00_temp;
    kalman->P[1][1] -= K[1] * P01_temp;
    
    return kalman->angle;
}