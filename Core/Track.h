#ifndef __TRACK_H
#define __TRACK_H
#include "Motor.h"
struct BoatState{
    float M;            //扭矩
    float An;           //法向加速度
    float At;           //切向加速度
};

void Track_Init();
struct BoatState Track_Update();
struct Motor_State Track_Control(struct BoatState state);
#endif