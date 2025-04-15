#ifndef __TRACK_H
#define __TRACK_H
#include "Motor.h"

struct BoatState
{
    float M;  // 扭矩
    float An; // 法向加速度
    float At; // 切向加速度
};
//和传感器板保持一致
struct Posture
{
    float position[2]; // 相对于自建坐标系
    float velocity[2]; // 线速度 自然坐标系
    // TODO:考虑要不要传输全局坐标系下的速度 就是一个变换的事情 但是也许可以利用传感器板子这边更丰富的数据减小误差
    float attitude[3];         // 姿态 三轴倾角 也分别是俯仰角 横滚角 偏航角 轴和角速度一样
    float angular_velocity[3]; // 角速度 分别是俯仰角 横滚角 偏航角  前两者都是以水平为0点
                               // 偏航角理论来说以任意一个方向是0点都行 然后只有正数 逆时针递增 转一圈归零
};
void Track_Init();
struct BoatState Track_Update(struct Posture p);
struct Motor_State Track_Control(struct BoatState state);
#endif