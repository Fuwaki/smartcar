#include "Fusion.h"
#include "Common.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
//------------------------Mahony参数
float twoKp=0.1; // 2 * proportional gain (Kp)
float twoKi=0.1; // 2 * integral gain (Ki)
//------------------------
//------------Mahony变量-----
float q0=0.0, q1=0.0, q2=0.0, q3=0.0;                        // quaternion of sensor frame relative to auxiliary frame
float integralFBx, integralFBy, integralFBz; // integral error terms scaled by Ki
float roll, pitch, yaw;
float grav[3];
float corrected_gx, corrected_gy, corrected_gz; // 修正后的角速度
float dt=0.1;                                       // 采样步长 为频率的倒数
//-----------------------

//-----------Kalman参数-------
float var_at, var_an;       // 加速度计的协方差
float var_ve;               // 编码器的协方差
float var_gps_x, var_gps_y; // gps的协方差
// 越大代表着对数据越不相信
//---------------------------
//-----------Kalman变量--------
float X[4];    // 状态
float P[4][4]; // 协方差矩阵
//---------------------------
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
void Mahony_Get_Angular_Velocity(float *gx, float *gy, float *gz)
{
    *gx = corrected_gx;
    *gy = corrected_gy;
    *gz = corrected_gz;
}

// void Kalman_Init()
// {
//     // 虽然暂时好像想不到什么需要提前计算而且计算一次就可以yiviysd
// }
// 两个共用的预测步骤
// static void Kalman_Predict(float omega, float an, float at, float dt)
// {
//     int i, j;
//     // 状态转移矩阵
//     float A[4][4];
//     // Initialize A matrix with dt values
//     A[0][0] = 1.0f; A[0][1] = 0.0f; A[0][2] = dt;   A[0][3] = 0.0f;
//     A[1][0] = 0.0f; A[1][1] = 1.0f; A[1][2] = 0.0f; A[1][3] = dt;
//     A[2][0] = 0.0f; A[2][1] = 0.0f; A[2][2] = 1.0f; A[2][3] = 0.0f;
//     A[3][0] = 0.0f; A[3][1] = 0.0f; A[3][2] = 0.0f; A[3][3] = 1.0f;
    
//     // 控制输入矩阵
//     float cos_omega = cos(omega);
//     float sin_omega = sin(omega);
//     float B[4][2];
//     // Initialize B matrix with dt values
//     B[0][0] = 0.5f * dt * dt * cos_omega; B[0][1] = -0.5f * dt * dt * sin_omega;
//     B[1][0] = 0.5f * dt * dt * sin_omega; B[1][1] = 0.5f * dt * dt * cos_omega;
//     B[2][0] = dt * cos_omega;             B[2][1] = -dt * sin_omega;
//     B[3][0] = dt * sin_omega;             B[3][1] = dt * cos_omega;
    
//     float X_pred[4];
//     float AP[4][4], APAt[4][4];
//     float AT[4][4];
//     float Q[4][4];
//     float BQBt[4][4], BT[2][4];
//     float tmp[4][2];
//     // 状态预测 X_pred=Ax+Bu
//     MAT_MUL(X_pred, A, X, 4, 4, 1);
//     for (i = 0; i < 4; i++)
//     {
//         X_pred[i] += B[i][0] * at + B[i][1] * an;
//     }
//     // 计算过程噪声矩阵
//     MAT_TRANSPOSE(BT, B, 4, 2);
//     MAT_MUL(tmp, B, (float[2][2]){{var_at, 0}, {0, var_an}}, 4, 2, 2)
//     MAT_MUL(BQBt, tmp, BT, 4, 2, 4);
//     for (int i = 0; i < 4; i++)
//         for (int j = 0; j < 4; j++)
//             Q[i][j] = BQBt[i][j];

//     // 先验协方差预测 P_[k]=AP[k-1]AT+Q
//     MAT_MUL(AP, A, P, 4, 4, 4);
//     MAT_TRANSPOSE(AT, A, 4, 4);
//     MAT_MUL(APAt, AP, AT, 4, 4, 4);
//     for (int i = 0; i < 4; i++)
//         for (int j = 0; j < 4; j++)
//             P[i][j] = APAt[i][j] + Q[i][j];

//     memcpy(X, X_pred, sizeof(float) * 4);
// }
// // 两个加速度计输入 ，然后是编码器测到的速度
// // omega方向角
// // 记得确保输入有效哦
// void Kalman_Update(float an, float at, float ve, float omega)
// {
//     // 执行预测
//     Kalman_Predict(omega, an, at, dt);
//     int i, j, k; // 矩阵运算的宏要用

//     // 编码器更新
//     float cos_omega = cosf(omega);
//     float sin_omega = sinf(omega);
//     // 观测矩阵
//     float H[1][4] = {{0, 0, cos_omega, sin_omega}};
//     float HT[4][1], PHt[4][1], HPHt[1][1];
//     float K[4][1]; // Kalman增益

//     // 计算卡尔曼增益
//     MAT_TRANSPOSE(HT, H, 1, 4)
//     MAT_MUL(PHt, P, HT, 4, 4, 1)
//     MAT_MUL(HPHt, H, PHt, 1, 4, 1)
//     HPHt[0][0] += var_ve;
//     for (i = 0; i < 4; i++)
//         K[i][0] = PHt[i][0] / HPHt[0][0];
//     {
//         // 使用增益进行状态更新
//         // X[k]=X_p[k]+K_k(z[k]-HX_p[k])
//         float hx = H[0][0] * X[0] + H[0][1] * X[1] + H[0][2] * X[2] + H[0][3] * X[3];
//         float innov = ve - hx; // 编码器测量残差
//         for (int i = 0; i < 4; i++)
//             X[i] += K[i][0] * innov;
//     }

//     {
//         // 协方差更新
//         float KH[4][4];
//         float I[4][4] = {
//             {1, 0, 0, 0},
//             {0, 1, 0, 0},
//             {0, 0, 1, 0},
//             {0, 0, 0, 1}};
//         float P_new[4][4];
//         // TODO:I需要是一个单位矩阵 不知道这个语法keil能不能正确初始化 
//         //TODO 请使用矩阵init而不是自己手写——Unikozera
//         MAT_MUL(KH, K, H, 4, 1, 4)
//         for (i = 0; i < 4; i++)
//             for (j = 0; j < 4; j++)
//                 I[i][j] -= KH[i][j];

//         MAT_MUL(P_new, I, P, 4, 4, 4)
//         memcpy(P, P_new, sizeof(float) * 16); // 把更新后的协方差保存
//     }
// }
// // gps获取的位置
// void Kalman_Update_GPS(float an, float at, float omega, float x, float y)
// {
//     // 执行预测
//     Kalman_Predict(omega, an, at, dt);

//     // GPS观测矩阵
//     float H[2][4] = {{1, 0, 0, 0}, {0, 1, 0, 0}};
//     float HT[4][2], PHt[4][2], HPHt[2][2];
//     // 用于计算的中间变量
//     float det, inv_det;
//     float HPHt_inv[2][2];
//     float R_gps[2][2] = {{var_gps_x, 0}, {0, var_gps_y}}; // gps观测协方差矩阵 这里假设x y轴噪声独立
//     float K[4][2];                                        // 增益
//     // 观测残差
//     float innov[2] = {
//         x - X[0],
//         y - X[1]};
//     float KH[4][4];
//     float I[4][4] = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}; // 单位矩阵

//     // 计算卡尔曼增益K
//     MAT_TRANSPOSE(HT, H, 2, 4)
//     MAT_MUL(PHt, P, HT, 4, 4, 2)
//     MAT_MUL(HPHt, H, PHt, 2, 4, 2)
//     for (int i = 0; i < 2; i++)
//         for (int j = 0; j < 2; j++)
//             HPHt[i][j] += R_gps[i][j];

//     // 2x2矩阵求逆
//     det = HPHt[0][0] * HPHt[1][1] - HPHt[0][1] * HPHt[1][0];
//     inv_det = 1.0 / det;
//     if (fabs(det) < 1e-6)
//         return; // 行列式接近零，退出
//     float HPHt_inv[2][2] = {
//         {HPHt[1][1] * inv_det, -HPHt[0][1] * inv_det},
//         {-HPHt[1][0] * inv_det, HPHt[0][0] * inv_det}};
//     MAT_MUL(K, PHt, HPHt_inv, 4, 2, 2)
//     for (int i = 0; i < 4; i++)
//         X[i] += K[i][0] * innov[0] + K[i][1] * innov[1];

//     // 协方差更新
//     MAT_MUL(KH, K, H, 4, 2, 4)
//     for (int i = 0; i < 4; i++)
//         for (int j = 0; j < 4; j++)
//             I[i][j] -= KH[i][j];
//     {
//         float P_new[4][4];
//         MAT_MUL(P_new, I, P, 4, 4, 4)
//         memcpy(P, P_new, sizeof(float) * 16);
//     }
// }
// void Kalman_Get_Data(float *x, float *y, float *vx, float *vy)
// {
//     *x = X[0];
//     *y = X[1];
//     *vx = X[2];
//     *vy = X[3];
// }
// // 用于获取一些kalman调试数据
// void Kalman_Debug()
// {
//     // TODO:Complete this
// }