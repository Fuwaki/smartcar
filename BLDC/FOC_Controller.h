#ifndef __FOC_CONTROLLER_H__
#define __FOC_CONTROLLER_H__
    void Set_PWM(double Ua, double Ub, double Uc);
    void OutPutter(double Uq, double Ud, double angle_elsped);
    double _electric_Angle(double shaft_angle, int pole_pairs);
    double _normalizeAngle(double angle);
    void getMotorInitAngle();
    void velocityOpenloop(double target_velocity);
    void positionCloseLoop(double target_position);
    void speedCloseLoop(double target_speed);
    extern double Ua, Ub, Uc;
#endif