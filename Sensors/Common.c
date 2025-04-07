#include "Common.h"
#include <math.h>
float fast_sqrt(float x) // 速度更快的平方根计算
{
	float halfx = 0.5f * x;
	float y = x;
	long i = *(long *)&y;
	// 0x5f3759df是一个平方根倒数速算法
	i = 0x5f3759df - (i >> 1);
	y = *(float *)&i;
	y = y * (1.5f - (halfx * y * y));
	return y;
}

struct EulerAngle QuaternionToEuler(struct Quaternion *e)
{
	struct EulerAngle angle;
	float norm;
	float q0, q1, q2, q3;
	float q0q0, q0q1, q0q2, q0q3;
	float q1q1, q1q2, q1q3;
	float q2q2, q2q3;
	float q3q3;
	float sinp;
	float sinr_cosp, cosr_cosp;
	float siny_cosp, cosy_cosp;

	// 对四元数进行归一化处理，确保单位四元数
	norm = sqrt(e->q0 * e->q0 + e->q1 * e->q1 + e->q2 * e->q2 + e->q3 * e->q3);

	// 如果四元数为零向量，返回零角度
	if (norm < 1e-10f)
	{
		angle.pitch = 0.0f;
		angle.roll = 0.0f;
		angle.yaw = 0.0f;
		return angle;
	}

	// 归一化四元数
	q0 = e->q0 / norm;
	q1 = e->q1 / norm;
	q2 = e->q2 / norm;
	q3 = e->q3 / norm;

	// 计算常用乘积，避免重复计算
	q0q0 = q0 * q0;
	q0q1 = q0 * q1;
	q0q2 = q0 * q2;
	q0q3 = q0 * q3;
	q1q1 = q1 * q1;
	q1q2 = q1 * q2;
	q1q3 = q1 * q3;
	q2q2 = q2 * q2;
	q2q3 = q2 * q3;
	q3q3 = q3 * q3;

	// 计算俯仰角 pitch (绕X轴旋转)
	// 使用atan2可以处理除数为零的情况
	sinp = 2.0f * (q0q2 - q1q3);

	// 处理万向节锁问题 - 当sinp接近±1时
	if (fabs(sinp) >= 0.999f)
	{
		angle.pitch = (sinp > 0) ? 90.0f : -90.0f; // 限制在±90度
	}
	else
	{
		angle.pitch = asin(sinp) * 57.295779513f; // 弧度转度
	}

	// 计算横滚角 roll (绕Y轴旋转)
	sinr_cosp = 2.0f * (q0q1 + q2q3);
	cosr_cosp = 1.0f - 2.0f * (q1q1 + q2q2);
	angle.roll = atan2(sinr_cosp, cosr_cosp) * 57.295779513f;

	// 计算偏航角 yaw (绕Z轴旋转)
	siny_cosp = 2.0f * (q0q3 + q1q2);
	cosy_cosp = 1.0f - 2.0f * (q2q2 + q3q3);
	angle.yaw = atan2(siny_cosp, cosy_cosp) * 57.295779513f;

	// 确保角度在合理范围内
	// roll和yaw的atan2已经保证在±180度范围内
	// pitch已经通过asin限制在±90度范围内

	return angle;
}