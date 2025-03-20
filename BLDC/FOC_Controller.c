#include <STC32G.H>
#include <intrins.h>
#include <math.h>
#include "PWM_Controller.h"
#include "Observer.h"
#include "PID_Controller.h"
#include "Lowpass_Filter.h"
#include "ADC.h"

#define PI 3.14159265358979323846 // 这么长怎么你了！

#define _constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))
#define _getCurr(value) (value * 114514 / 4096 - 57.257) // idk awa

#pragma region Motor_Parameters
float voltage_power_supply = 12; // 12v??
double Electric_Angle;
double Shaft_Angle = 0;   // 闭环所用的变量
double Initial_Angle = 0; // 有可能转子不在0度位置，所以需要一个初始角度
double Ualpha;
double Ubeta;
double Ua;
double Ub;
double Uc;
double DC_a;
double DC_b;
double DC_c;
float Iu;
float Iv;
float Iw;
float Ialpha;
float Ibeta;
double Uq;
float Id;
float Iq;
#pragma endregion

struct info MotorVariables;

double _normalizeAngle(double angle) // 控制在0~2π之间 LOL
{
    double a = fmod(angle, 2 * PI);
    return a >= 0 ? a : (a + 2 * PI);
}

void getMotorInitAngle()
{
    Initial_Angle = MotorVariables.motor_postion;
}

double _electric_Angle(double shaft_angle, int pole_pairs) // 电机的电角度 = 机械角度 * 极对数
{
    return _normalizeAngle((MotorVariables.motor_direction * pole_pairs * shaft_angle) - Initial_Angle);
}

void Set_PWM(double Ua, double Ub, double Uc)
{
    DC_a = _constrain(Ua / voltage_power_supply, 0.0f, 1.0f);
    DC_b = _constrain(Ub / voltage_power_supply, 0.0f, 1.0f);
    DC_c = _constrain(Uc / voltage_power_supply, 0.0f, 1.0f);

    Set_PWM_Duty(0, (unsigned char)((DC_a) * 100));
    Set_PWM_Duty(1, (unsigned char)((DC_b) * 100));
    Set_PWM_Duty(2, (unsigned char)((DC_c) * 100));
}

void OutPutter(double Uq, double Ud, double angle_el) // Ud暂时不知道2333,貌似影响不大,Uq是主要的，Ud直轴电压
{
    angle_el = _normalizeAngle(angle_el); // 物理上，而不是理论上

    // Park变换: 旋转坐标系(d-q)到静止坐标系(alpha-beta)
    Ualpha = Ud * cos(angle_el) - Uq * sin(angle_el);
    Ubeta = Ud * sin(angle_el) + Uq * cos(angle_el);

    /* Anti-Clarke变换 */
    Ua = Ualpha + voltage_power_supply / 2; // 电压被平移到中间去玩
    Ub = (sqrt(3) * Ubeta - Ualpha) / 2 + voltage_power_supply / 2;
    Uc = (-Ualpha - sqrt(3) * Ubeta) / 2 + voltage_power_supply / 2;

    Set_PWM(Ua, Ub, Uc); // 还是在这里做转化吧
}

void velocityOpenloop(double target_velocity) // finally...
{
    /* 使用早前设置的voltage_power_supply的1/3作为Uq值，这个值会直接影响输出力矩
    最大只能设置为Uq = voltage_power_supply/2，否则ua,ub,uc会超出供电电压限幅 */
    Uq = voltage_power_supply / 2.001f;

    Shaft_Angle = _normalizeAngle(Shaft_Angle + target_velocity); // 开环控制，软件++
    OutPutter(Uq, 0.0, Shaft_Angle);                              // 输出电压
}

void positionCloseLoop(double target_position) // 位置闭环控制 位置单位为rad
{
    OutPutter(PID_Controller(target_position - MotorVariables.motor_direction * MotorVariables.motor_postion), 0.0, _electric_Angle(MotorVariables.motor_postion, 7));
}

void speedCloseLoop(double target_speed) // 速度闭环控制 速度单位为rad/s
{
    OutPutter(PID_Controller((target_speed - MotorVariables.angular_speed)), 0.0, _electric_Angle(MotorVariables.motor_postion, 7));
}

float calCurrent(double angle_el) // 计算电流
{
    // 获取ADC原始值
    unsigned int adc_u, adc_v, adc_w;
    int offset = 2048;     // ADC零点偏移，12位ADC中点值
    float scale = 0.0806f; // 电流转换系数, 根据实际电流传感器校准

    // 获取三相电流的ADC原始值
    adc_u = Get_U_Current();
    adc_v = Get_V_Current();
    adc_w = Get_W_Current();

    // 将ADC原始值转换为实际电流值(A)
    Iu = (adc_u - offset) * scale;
    Iv = (adc_v - offset) * scale;
    Iw = (adc_w - offset) * scale;

    // Clarke变换: 三相(abc)到二相静止坐标系(alpha-beta)
    // 假设三相电流和为零: Iu + Iv + Iw = 0
    Ialpha = Iu;
    Ibeta = (Iu + 2 * Iv) / sqrt(3);

    // Park变换: 静止坐标系(alpha-beta)到旋转坐标系(d-q)
    Id = Ialpha * cos(angle_el) + Ibeta * sin(angle_el);
    Iq = -Ialpha * sin(angle_el) + Ibeta * cos(angle_el);

    return Iq;
}

void currentCloseLoop(double target_current) // 电流闭环控制 电流单位为A
{
    OutPutter(PID_Controller(target_current - calCurrent(_electric_Angle(MotorVariables.motor_postion, 7))), 0.0, _electric_Angle(MotorVariables.motor_postion, 7));
}