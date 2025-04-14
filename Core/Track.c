#include "Track.h"
#include "PID.h"
#include "config.h"
#define BASE_SPEED 0.5

#define LOWEST_SPEED 0.2

#define CONSTRAIN(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

static PID_Control v_pid;  // 切向速度环
static PID_Control m_pid;  // 角位置环 输出目标角速度
static PID_Control m2_pid; // 角速度环 输出角加速度
static PID_Control n_pid;  // 法向位置环 输出法向速度
static PID_Control n2_pid; // 法向速度环 输出法向加速度

void Track_Init()
{
    PID_Init(&v_pid, 1.0, 0.0, 0.0);
    PID_Init(&m_pid, 1.0, 0.0, 0.0);
    PID_Init(&m2_pid, 1.0, 0.0, 0.0);
    PID_Init(&n_pid, 1.0, 0.0, 0.0);
    PID_Init(&n2_pid, 1.0, 0.0, 0.0);
}

// 输出法向加速度
float Track_Normal(float normal_error, float normal_velocity)
{
    float target_normal_v = PID_Update(&n_pid, normal_error);
    float u = PID_Update(&n2_pid, target_normal_v - normal_velocity);
    return CONSTRAIN(u, 0.0, 1.0);
}
// 输出切向加速度
float Track_Speed(float target_velocity, float now_velocity)
{
    float u = PID_Update(&v_pid, target_velocity - now_velocity);
    // TODO: 想一个策略处理负转速 要和Track_Control相匹配
    return CONSTRAIN(u, LOWEST_SPEED, 1.0);
}
// 输出角加速度（扭矩）
float Track_Angle(float angle_error, float angle_velocity)
{
    float target_angular_v = PID_Update(&m_pid, angle_error);

    float u = PID_Update(&m2_pid, angle_velocity - target_angular_v);

    return CONSTRAIN(u, 0.0, 1.0);
}
struct BoatState Track_Update()
{
    struct BoatState state;

    state.An = Track_Normal(); // 法向加速度
    state.At = Track_Speed();  // 切向加速度
    state.M = Track_Angle();   // 扭矩
    return state;
}
struct Motor_State Track_Control(struct BoatState state)
{
    struct Motor_State motor_state;
    if (state.An > 0)
    {
        // 左转
        motor_state.right = BASE_SPEED + state.An;
        motor_state.left = BASE_SPEED;
    }
    else
    {
        // 右转
        motor_state.left = BASE_SPEED - state.An;
        motor_state.right = BASE_SPEED;
    }
    motor_state.back_left = state.At - state.M;
    motor_state.back_right = state.At + state.M;
    // 不可以小于最低速度以防止电机停转
    motor_state.back_left = motor_state.back_left > LOWEST_SPEED ? motor_state.back_left : LOWEST_SPEED;
    motor_state.back_right = motor_state.back_right > LOWEST_SPEED ? motor_state.back_right : LOWEST_SPEED;
    return motor_state;
}
