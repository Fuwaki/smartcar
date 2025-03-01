#ifndef __OBSERVER_H__
#define __OBSERVER_H__
    struct info
    {
        float motor_postion; //电机位置
        float angular_speed; //电机角速度
    };

    void Init_Observer();
    struct info Update_Observer(float A,float B,float C);
#endif