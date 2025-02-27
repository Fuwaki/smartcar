#ifndef __FOC_CONTROLLER_H__
#define __FOC_CONTROLLER_H__
    void Set_PWM(double Ua, double Ub, double Uc);
    void OutPutter(double Uq, double Ud, double angle_elsped);
    double Electric_Angle(double shaft_angle, int pole_pairs);
    double _normalizeAngle(double angle);
    double velocityOpenloop(double target_velocity)
    // extern double Electric_Angle;
    // extern double Shaft_Angle;
    // extern double Ualpha;
    // extern double Ubeta;
    // extern double Ua;
    // extern double Ub;
    // extern double Uc;
    // extern double DC_a;
    // extern double DC_b;
    // extern double DC_c;
    // extern float voltage_power_supply;
#endif