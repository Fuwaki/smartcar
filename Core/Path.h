#ifndef PATH_H
#define PATH_H
#include "Math.h"
enum POINT_EFFECT{
    PointEffect_None,
    PointEffect_STOP,
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
    unsigned int CurrentIndex;
    //FIXME:这里就不处理点的数量可能超过最大数量了
}Path;

void Path_Init(Path *path,enum PATH_TYPE path_type);
void Path_AppendPoint(Path *path, Point2D point);
void Path_SetType(Path *path, enum PATH_TYPE path_type);
void Path_InsertPoint(Path *path, int index, Point2D point);            //也许之后会用到动态路径规划 但是现在就不实现了 留个点子在这
void Path_Update(Path *path,Point2D * now_position);
void Path_GetDirection(Path* path,float *angular_deviation,Point2D *way_to_path);

#endif