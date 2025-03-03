#include <STC32G.H>
#include <intrins.h>
#include <math.h>
#include "PWM_Controller.h"
#include "Observer.h"
#include "Lowpass_Filter.h"

#define PI 3.14159265358979323846 //这么长怎么你了！

#define _constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

#pragma region Motor_Parameters
float voltage_power_supply = 12;//12v??
double Electric_Angle;
// double Shaft_Angle; 闭环所用的变量
double Initial_Angle = 0; //有可能转子不在0度位置，所以需要一个初始角度
double Ualpha;
double Ubeta;
double Ua;
double Ub;
double Uc;
double DC_a;
double DC_b;
double DC_c;
double Uq;
#pragma endregion

struct info MotorVariables;

double _normalizeAngle(double angle) //控制在0~2π之间 LOL
{
    double a = fmod(angle, 2*PI);
    return a >= 0 ? a : a + 2*PI;
}

double _electric_Angle(double shaft_angle, int pole_pairs) //电机的电角度 = 机械角度 * 极对数
{
    return _normalizeAngle((double) MotorVariables.motor_direction * pole_pairs * shaft_angle - Initial_Angle);
}

void Set_PWM(double Ua, double Ub, double Uc)
{
    DC_a = _constrain(Ua/voltage_power_supply, 0.0f, 1.0f);
    DC_b = _constrain(Ub/voltage_power_supply, 0.0f, 1.0f);
    DC_c = _constrain(Uc/voltage_power_supply, 0.0f, 1.0f);

    Set_PWM_Duty(0, (unsigned char)(DC_a * 100));
    Set_PWM_Duty(1, (unsigned char)(DC_b * 100));
    Set_PWM_Duty(2, (unsigned char)(DC_c * 100));
}

void OutPutter(double Uq, double Ud, double angle_el) //Ud暂时不知道2333，貌似影响不大，但是留着.Uq是主要的，Ud直轴电压，什么？为什么叫_elsped?LOL LUA
{
    angle_el = _normalizeAngle(angle_el); //物理上，而不是理论上

    /* Park变换 */
    Ualpha = -Uq * sin(angle_el) + Ud * cos(angle_el); 
    Ubeta = Uq * cos(angle_el) + Ud * sin(angle_el); 

    /* Anti-Clarke变换 */
    Ua = Ualpha + voltage_power_supply/2; //电压被平移到中间去玩
    Ub = (sqrt(3)*Ubeta-Ualpha)/2 + voltage_power_supply/2;
    Uc = (-Ualpha-sqrt(3)*Ubeta)/2 + voltage_power_supply/2;
    
    Set_PWM(Ua,Ub,Uc); //还是在这里做转化吧
}

// double velocityOpenloop(double target_velocity) //finally...
// {
//     /* 使用早前设置的voltage_power_supply的1/3作为Uq值，这个值会直接影响输出力矩
//     最大只能设置为Uq = voltage_power_supply/2，否则ua,ub,uc会超出供电电压限幅 */
//     Uq = voltage_power_supply/3;

//     Shaft_Angle = _normalizeAngle(Shaft_Angle + target_velocity*dt); //开环控制，软件++

//     OutPutter(Uq, 0.0, _electric_Angle(Shaft_Angle, 1));
//     return Uq;
// }

void positionCloseLoop(double target_position)
{
    OutPutter(_constrain((target_position - MotorVariables.motor_direction*MotorVariables.motor_postion)*180/PI,-6,6), 0.0, _electric_Angle(MotorVariables.motor_direction, 1));
}