#ifndef __MAIN_H__
#define __MAIN_H__
void Start();
void AddCurrentPositionAsPathPoint();
void AddCurrentPositionAsStopPoint();
void Stop();
void ClearPath();
extern float launch_progress; // 启动进度
#endif /* __MAIN_H__ */