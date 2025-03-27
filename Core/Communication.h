#ifndef QWQ
#define QWQ
struct Point{
    float x;
    float y;
};
struct SensorInitFrame{
    int size;
    struct Point points[];
};

struct SensorFrame{
    bool alive;         //传感器是否正常工作
    
    
};
struct CameraFrame{
    bool alive;         //相机是否正常工作
    bool available;     //是否找到线
    float error;        //误差
};

#endif