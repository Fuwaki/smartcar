#ifndef __LOWPASS_FILTER_H__
#define __LOWPASS_FILTER_H__

// 简单的一阶低通滤波函数
double Lowpass_Filter(double x,int channel);

// // 可配置的一阶指数低通滤波器
// double Exp_Lowpass_Filter(double new_value, double alpha);

// // 设置指数低通滤波器的平滑系数 (0.0 < alpha < 1.0)
// // alpha越小，滤波效果越强，但响应越慢
// void Set_Filter_Alpha(double alpha);

// // 重置所有滤波器
// void Reset_Filters(void);

// // 滑动平均低通滤波器
// // window_size: 滑动窗口大小
// double MA_Lowpass_Filter(double new_value, int channel, int window_size);


#endif