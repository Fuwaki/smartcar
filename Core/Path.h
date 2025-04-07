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


#endif