#ifndef __COMMON_H__
#define __COMMON_H__
    float fast_sqrt(float x);

    //四元数
    struct Quaternion{
        float q0,q1,q2,q3;
    };
    //欧拉角
    struct EulerAngle{
        float pitch,roll,yaw;
    };
    struct EulerAngle QuaternionToEuler(struct Quaternion* e);

#endif