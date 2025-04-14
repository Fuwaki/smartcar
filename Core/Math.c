#include "Math.h"

float Vec_Norm(Point2D vector)
{
    return sqrt(vector.x * vector.x + vector.y * vector.y);
}
float Vec_Dot(Point2D vector1, Point2D vector2)
{
    return sqrt(vector1.x * vector2.x + vector1.y * vector2.y);
}
Point2D Vec_Add(Point2D a, Point2D b)
{
    Point2D result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}
Point2D Vec_Mul(Point2D vector, float k)
{
    Point2D result;
    result.x = vector.x * k;
    result.y = vector.y * k;
    return result;
} // ³Ë

Point2D Vec_Sub(Point2D a, Point2D b)
{
    Point2D result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;

} // Ïà¼õ aÎª±»¼õ
Point2D Cross(Point2D vector1, Point2D vector2)
{
    Point2D result;
    result.x = vector1.x * vector2.y - vector1.y * vector2.x;
    result.y = vector1.x * vector2.y - vector1.y * vector2.x;
    return result;

} // ²æ³Ë

float signf(float x)
{
    if (x > 0)
        return 1.0;
    else if (x < 0)
        return -1.0;
    else
        return 0.0;

} // ·ûºÅº¯Êı