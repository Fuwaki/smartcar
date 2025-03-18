#ifndef __GPS_H__
#define __GPS_H__
/*
初始化时以当前点建立坐标系 当前朝向为y轴正方向
*/

//姿态信息
struct Posture{
    float angular_velocity[3];      //角速度 分别是俯仰角 横滚角 偏航角  前两者都是以水平为0点 偏航角理论来说以任意一个方向是0点都行 然后只有正数 顺时针递增 转一圈归零
    float velocity[3];              //线速度
    float attitude[3];              //姿态 三轴倾角 也分别是俯仰角 横滚角 偏航角 轴和角速度一样
    float position[2];              //相对于自建坐标系
};

struct Posture get_fusion_data();
#endif