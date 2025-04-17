#include "Track.h"
#include "PID.h"
#include "config.h"
#include "math.h"

#define CONSTRAIN(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

static PID_Control v_pid;  // 切向速度环
static PID_Control m_pid;  // 角位置环 输出目标角速度
static PID_Control m2_pid; // 角速度环 输出角加速度
static PID_Control n_pid;  // 法向位置环 输出法向速度
static PID_Control n2_pid; // 法向速度环 输出法向加速度

void Track_Init(Path *path)
{

    PID_Init(&v_pid, 1.0, 0.0, 0.0);
    PID_Init(&m_pid, 1.0, 0.0, 0.0);
    PID_Init(&m2_pid, 1.0, 0.0, 0.0);
    PID_Init(&n_pid, 1.0, 0.0, 0.0);
    PID_Init(&n2_pid, 1.0, 0.0, 0.0);
}

// 输出法向加速度
float Track_Normal(float normal_error, float normal_velocity, float dt)
{
    float target_normal_v = PID_Update(&n_pid, normal_error, dt);
    float u = PID_Update(&n2_pid, target_normal_v - normal_velocity, dt);
    return CONSTRAIN(u, 0.0, 1.0);
}
// 输出切向加速度
float Track_Speed(float target_velocity, float now_velocity, float dt)
{
    float u = PID_Update(&v_pid, target_velocity - now_velocity, dt);
    // TODO: 想一个策略处理负转速 要和Track_Control相匹配
    return u;
}
// 输出角加速度（扭矩）
float Track_Angle(float angle_error, float angle_velocity, float dt)
{
    float target_angular_v = PID_Update(&m_pid, angle_error, dt);

    float u = PID_Update(&m2_pid, angle_velocity - target_angular_v, dt);

    return CONSTRAIN(u, 0.0, 1.0);
}
float get_angle_error(float target, float now)
{
    float error;
    target = fmod(target, 360.0f);
    now = fmod(now, 360.0f);
    error = target - now;
    if (error > 180.0f)
    {
        error -= 360.0f;
    }
    else if (error < -180.0f)
    {
        error += 360.0f;
    }
    return error;
}
struct BoatState Track_Update(struct Posture p, float dt)
{
    struct BoatState state;
    float target_direction;
    // 先更新位置
    Point2D now_position;
    now_position.x = p.position[0];
    now_position.y = p.position[1];
    Path_Update(&now_position);
    // 获取目标方向
    target_direction = Path_GetDirection();

    state.An = 0.0; // 法向加速度       //TODO:未来使用6电机策略
    state.M = Track_Angle(get_angle_error(target_direction, p.attitude[2]), p.angular_velocity[2], dt); // 扭矩

    state.At = 0.5; // 开环加速
    return state;
}

#define BASE_SPEED 0.5
#define LOWEST_SPEED 0.2
// 在这个函数完成运动学模型的转换
struct Motor_State Track_ToMotorState(struct BoatState state)
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
