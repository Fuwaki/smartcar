#include <STC32G.H>
#include <intrins.h>
//--------------电机参数-----------------
#define R 10  // 电阻
#define L 100 // 电感

//--------------观测器参数----------------
#define K 0.5     // 滑膜增益
#define LPF_k 0.1 // 低通滤波器截止系数
#define dt 0.0001 // 微小的时间 控制周期

//--------------观测器缓存----------------
// 绝对坐标系
float I_x_est = 0.0; // x轴预测电流
float I_y_est = 0.0; // y轴预测电流
float E_x_est = 0.0; // x轴预测反电动势
float E_y_est = 0.0; // y轴预测反电动势

//--------------滤波器缓存----------------

// 用个一阶线性滤波算了
// TODO: 可能更换
float LPF(float value, float previous_value)
{
    return LPF_k * value + (1.0 - LPF_k);
}

struct info
{
    float motor_postion; // 电机位置
    float angular_speed; // 电机角速度
};

void Init_Observer()
{
    // Init_Observer();
}
// TODO:换成sat函数以抑制高频抖动
float sign(float input)
{
}
// 绝对坐标系下的电压电流
struct info Update_Observer(float Ix, float Iy, float Ux, float Uy)
{
    // 获得误差
    float error_x = Ix - I_x_est;
    float error_y = Iy - I_y_est;

    // 滑模量
    float s_x = K * sign(error_x);
    float s_y = K * sign(error_y);

    // 使用电机模型来更新估计参数
    // di/dt=(1/L)(u-R*i-e)      其中e为反电动势 u为电压
    float delta_i_x_est = (Ux - R * Ix - E_x_est) / L + s_x;
    float delta_i_y_est = (Ux - R * Ix - E_y_est) / L + s_y;

    // 更新电流
    I_x_est += delta_i_x_est * dt;
    I_y_est += delta_i_y_est * dt;

    // TODO：引入锁相环来实现更加精准的相位跟踪
}