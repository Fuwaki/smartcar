#include <STC32G.H>
#include <intrins.h>
#include <math.h> //TODO:这个好像用不了硬件三角函数加速器捏
#include "PWM_Controller.h"
//--------------电机参数-----------------
#define R 10  // 电阻
#define L 100 // 电感

//--------------观测器参数----------------
#define eK 0.5    // 电动势观测器增益
#define iK 0.5    // 电流观测器增益
#define LPF_k 0.1 // 低通滤波器截止系数
#define sK 0.2    // 转速估计参数

//--------------观测器缓存----------------
// 绝对坐标系
float I_x_est = 0.0; // x轴预测电流
float I_y_est = 0.0; // y轴预测电流
float E_x_est = 0.0; // x轴预测反电动势
float E_y_est = 0.0; // y轴预测反电动势

//--------------滤波器缓存----------------

// 用个一阶线性滤波算了
// TODO: 可能更换
float LPF(float value, float new_value)
{
    return LPF_k * value + (1.0 - LPF_k) * new_value;
}

struct info
{
    float motor_postion; // 电机位置
    float angular_speed; // 电机角速度
    int motor_direction; // 电机方向
};

// TODO:换成sat函数以抑制高频抖动
float sign(float input)
{
}

float arctan2(float,float)
{
    // arctan2函数未实现 //还是得泰勒公式!!!!!!!!!!
}
// 绝对坐标系下的电压电流
struct info Update_Observer(float Ix, float Iy, float Ux, float Uy)
{
    float angle, angular_speed, E_x_est_new, E_y_est_new;
    struct info i;
    // 获得误差
    float error_x = Ix - I_x_est;
    float error_y = Iy - I_y_est;

    // 滑模量
    float s_x = sign(error_x);
    float s_y = sign(error_y);

    // TODO:这个地方存在多种实现，要结合实验确定算法

    // 估计电流变化量
    // di/dt=(1/L)(u-R*i-e)      其中e为反电动势 u为电压
    float delta_i_x_est = (Ux - R * Ix - E_x_est + iK * s_x) / L;
    float delta_i_y_est = (Ux - R * Ix - E_y_est + iK * s_y) / L;

    // 更新电流 使用欧拉法可能引入误差 看具体情况考虑切换到SK4
    I_x_est += delta_i_x_est * dt;
		I_y_est += delta_i_y_est * dt;

    // 估计反电动势
    E_x_est_new = E_x_est + eK * s_x;
    E_y_est_new = E_y_est + eK * s_y;
    // （也许反电动势更新要放在更新电流之前，但是现在我也不好说）

    // 低通滤波可能要补偿相位延迟
    E_x_est = LPF(E_x_est, E_x_est_new);
    E_y_est = LPF(E_y_est, E_y_est_new);

    // 反电动势到手，算角度！
    angle = arctan2(-E_x_est, E_y_est); // 注意符号

    // 转速两个方法，一种是角度微分，一种是看反电动势大小，因为速度越快反电动势一定越快，暂时先选后面那种
    angular_speed = sqrt(E_x_est * E_x_est + E_y_est * E_y_est) / sK;

    // TODO：引入锁相环来实现更加精准的相位跟踪

    i.angular_speed = angular_speed;
    i.motor_postion = angle;
    i.motor_direction = (i.angular_speed > 0) ? 1 : -1;
    return i;
}