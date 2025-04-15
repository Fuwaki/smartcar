#ifndef __FUSION_H__
#define __FUSION__

/*
初始化时以当前点建立坐标系 当前朝向为y轴正方向
*/
void Mahony_Init();
void Mahony_Update(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);
void Mahony_Get_Gravity_Vector(float *x,float *y,float *z);           //获取重力的分量 使用加速度的时候需要减去重力
void Mahony_Get_Quaternion(float *w,float *x,float *y,float *z);
void Mahony_Get_Angular_Velocity(float *gx, float *gy, float *gz);

//姿态信息
struct Posture{
    float position[2];              //相对于自建坐标系
    float velocity[2];              //线速度 自然坐标系
    //TODO:考虑要不要传输全局坐标系下的速度 就是一个变换的事情 但是也许可以利用传感器板子这边更丰富的数据减小误差
    float attitude[3];              //姿态 三轴倾角 也分别是俯仰角 横滚角 偏航角 轴和角速度一样
    float angular_velocity[3];      //角速度 分别是俯仰角 横滚角 偏航角  前两者都是以水平为0点 偏航角理论来说以任意一个方向是0点都行 然后只有正数 逆时针递增 转一圈归零
};

// struct Posture get_fusion_data();
// void Kalman_Debug();
// void Kalman_Get_Data(float *x,float *y,float *vx,float *vy);
// void Kalman_Update_GPS(float an,float at,float omega,float x,float y);
// void Kalman_Update(float an,float at,float ve,float omega);
// void Kalman_Init();
#endif