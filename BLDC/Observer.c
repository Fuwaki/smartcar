#include <AI8051U.H>
#include <intrins.h>

struct info
{
    float motor_postion; //电机位置
    float angular_speed; //电机角速度
};

void Init_Observer()
{
    // Init_Observer();
}

struct info Update_Observer(float A,float B,float C)
{
    struct info temp;
    temp.motor_postion = A;
    temp.angular_speed = B;
    /*C自己写*/
    return temp;
}