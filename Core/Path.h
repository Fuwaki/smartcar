#ifndef PATH_H
#define PATH_H
#include "Math.h"
enum POINT_EFFECT{
    PointEffect_None,
    PointEffect_Stop,
};
typedef struct {
    Point2D* p0;
    Point2D* p1;
    Point2D* p2;
    Point2D* p3;
    enum POINT_EFFECT Effect;
}PointGroup;
enum PATH_TYPE{
    Curve,
    Straight
};
typedef struct {
    Point2D * PointList;
    int PointCount;
    enum PATH_TYPE PathType;
    //FIXME:这里就不处理点的数量可能超过最大数量了
    int MaxLength;
}Path;

void Path_Init(Path *path,Point2D* pointlist,int count,int maxlength,enum PATH_TYPE path_type);
void Path_AppendPoint(Path *path, Point2D point,enum POINT_EFFECT effect);
void Path_SetType(Path *path, enum PATH_TYPE path_type);
void Path_InsertPoint(Path *path, int index, Point2D point);            //也许之后会用到动态路径规划 但是现在就不实现了 留个点子在这


//启动某个路径
void Path_Select(Path* path);
void Path_Update(Point2D * now_position);
//这里解决的是需要小船以什么状态才能回到路径上 而具体怎么达到状态要看Track.c了
float Path_GetDirection();
float Path_GetNormalError();
int Path_Check();

#endif