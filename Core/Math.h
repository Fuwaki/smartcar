#ifndef MATH_H
#define MATH_H
#include <math.h>
typedef struct {
    float x;
    float y;
} Point2D;
//TODO :ʹ��ָ����Ϊ���������������� �������ӱ�д�����Ѷ� ������ѡ��ʹ��ָ��
float Vec_Norm(Point2D vector);     //ģ��
float Vec_Dot(Point2D vector1, Point2D vector2); //���
Point2D Vec_Add(Point2D a, Point2D b);            //���
Point2D Vec_Mul(Point2D vector, float k);           //��

Point2D Vec_Sub(Point2D a, Point2D b);            //��� aΪ����
Point2D Cross(Point2D vector1, Point2D vector2);        //���

float signf(float x);           //���ź���

#endif