#ifndef PATH_H
#define PATH_H
#include "Math.h"
typedef struct {
    Point2D* p0;
    Point2D* p1;
    Point2D* p2;
    Point2D* p3;
}PointGroup;

typedef struct {
    Point2D * PointList;
    int PointCount;

}Path;

void Path_Init(Path *path, Point2D *pointlist, int pointcount);
void Path_Update(Path *path,Point2D * now_position);
void Path_GetDirection(Path* path,float *angular_deviation,Point2D *way_to_path);

#endif