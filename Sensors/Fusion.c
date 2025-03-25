#include "Fusion.h"
#include "Common.h"
#include <math.h>
#include <stdio.h>
//------------------------要调的参数
float twoKp; // 2 * proportional gain (Kp)
float twoKi; // 2 * integral gain (Ki)
//------------------------
float q0, q1, q2, q3;                        // quaternion of sensor frame relative to auxiliary frame
float integralFBx, integralFBy, integralFBz; // integral error terms scaled by Ki
float roll, pitch, yaw;
float grav[3];
float corrected_gx, corrected_gy, corrected_gz; // 修正后的角速度
float dt;                                       // 采样步长 为频率的倒数

void Mahony_Init()
{
    q0 = 1.0f;
    q1 = 0.0f;
    q2 = 0.0f;
    q3 = 0.0f;
    integralFBx = 0.0f;
    integralFBy = 0.0f;
    integralFBz = 0.0f;
}
// 调用之前请检查数据有效性 磁场计或者加速度计的值都为0都会造成错误
// g陀螺仪 a加速度计 m磁场计
void Mahony_Update(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz)
{
    float recipNorm;
    float q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;
    float hx, hy, bx, bz;
    float halfvx, halfvy, halfvz, halfwx, halfwy, halfwz;
    float halfex, halfey, halfez;
    float qa, qb, qc;

    // Convert gyroscope degrees/sec to radians/sec
    gx *= 0.0174533f;
    gy *= 0.0174533f;
    gz *= 0.0174533f;

    // Normalise accelerometer measurement
    recipNorm = fast_sqrt(ax * ax + ay * ay + az * az);
    ax *= recipNorm;
    ay *= recipNorm;
    az *= recipNorm;

    // Normalise magnetometer measurement
    recipNorm = fast_sqrt(mx * mx + my * my + mz * mz);
    mx *= recipNorm;
    my *= recipNorm;
    mz *= recipNorm;

    // Auxiliary variables to avoid repeated arithmetic
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

    // Reference direction of Earth's magnetic field
    hx = 2.0f *
         (mx * (0.5f - q2q2 - q3q3) + my * (q1q2 - q0q3) + mz * (q1q3 + q0q2));
    hy = 2.0f *
         (mx * (q1q2 + q0q3) + my * (0.5f - q1q1 - q3q3) + mz * (q2q3 - q0q1));
    bx = fast_sqrt(hx * hx + hy * hy); // 原为sqrtf
    bz = 2.0f *
         (mx * (q1q3 - q0q2) + my * (q2q3 + q0q1) + mz * (0.5f - q1q1 - q2q2));

    // Estimated direction of gravity and magnetic field
    halfvx = q1q3 - q0q2;
    halfvy = q0q1 + q2q3;
    halfvz = q0q0 - 0.5f + q3q3;
    halfwx = bx * (0.5f - q2q2 - q3q3) + bz * (q1q3 - q0q2);
    halfwy = bx * (q1q2 - q0q3) + bz * (q0q1 + q2q3);
    halfwz = bx * (q0q2 + q1q3) + bz * (0.5f - q1q1 - q2q2);

    // Error is sum of cross product between estimated direction
    // and measured direction of field vectors
    halfex = (ay * halfvz - az * halfvy) + (my * halfwz - mz * halfwy);
    halfey = (az * halfvx - ax * halfvz) + (mz * halfwx - mx * halfwz);
    halfez = (ax * halfvy - ay * halfvx) + (mx * halfwy - my * halfwx);

    // Compute and apply integral feedback if enabled
    if (twoKi > 0.0f)
    {
        // integral error scaled by Ki
        integralFBx += twoKi * halfex * dt;
        integralFBy += twoKi * halfey * dt;
        integralFBz += twoKi * halfez * dt;
        gx += integralFBx; // apply integral feedback
        gy += integralFBy;
        gz += integralFBz;
    }
    else
    {
        integralFBx = 0.0f; // prevent integral windup
        integralFBy = 0.0f;
        integralFBz = 0.0f;
    }

    // Apply proportional feedback
    gx += twoKp * halfex;
    gy += twoKp * halfey;
    gz += twoKp * halfez;

    // 保存修正后的角速度
    corrected_gx = gx;
    corrected_gy = gy;
    corrected_gz = gz;

    // Integrate rate of change of quaternion
    gx *= (0.5f * dt); // pre-multiply common factors
    gy *= (0.5f * dt);
    gz *= (0.5f * dt);
    qa = q0;
    qb = q1;
    qc = q2;
    q0 += (-qb * gx - qc * gy - q3 * gz);
    q1 += (qa * gx + qc * gz - q3 * gy);
    q2 += (qa * gy - qb * gz + q3 * gx);
    q3 += (qa * gz + qb * gy - qc * gx);

    // Normalise quaternion
    recipNorm = fast_sqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    q0 *= recipNorm;
    q1 *= recipNorm;
    q2 *= recipNorm;
    q3 *= recipNorm;
}
void Mahony_Get_Gravity_Vector(float *x, float *y, float *z)
{
    *x = 2.0f * (q1 * q3 - q0 * q2);
    *y = 2.0f * (q0 * q1 + q2 * q3);
    *z = 2.0f * (q1 * q0 - 0.5f + q3 * q3);
}
void Mahony_Get_Quaternion(float *w, float *x, float *y, float *z)
{
    *w = q0;
    *x = q1;
    *y = q2;
    *z = q3;
}
void Mahony_Get_Angular_Velocity(float *roll, float *pitch, float *yaw)
{
    *roll = corrected_gx;
    *pitch = corrected_gy;
    *yaw = corrected_gz;
}