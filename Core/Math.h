#ifndef MATH_H
#define MATH_H
#include <math.h>
typedef struct {
    float x;
    float y;
} Point2D;
//TODO :使用指针作为运算参数会提高性能 但会增加编写代码难度 所以我选择不使用指针
float Vec_Norm(Point2D vector);     //模长
float Vec_Dot(Point2D vector1, Point2D vector2); //点积
Point2D Vec_Add(Point2D a, Point2D b);            //相加
Point2D Vec_Mul(Point2D vector, float k);           //乘

Point2D Vec_Sub(Point2D a, Point2D b);            //相减 a为被减
Point2D Cross(Point2D vector1, Point2D vector2);        //叉乘

float signf(float x);           //符号函数

#endif