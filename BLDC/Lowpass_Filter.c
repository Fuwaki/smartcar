#include "PWM_Controller.h"
#include "Lowpass_Filter.h"

// // 卡尔曼滤波器参数
// static double x_est = 0;     // 状态估计值
// static double p_est = 1;     // 估计误差协方差
// static double q = 0.01;      // 过程噪声协方差 较大的值使滤波器更快响应变化，但会增加输出噪声
// static double r = 0.1;       // 测量噪声协方差 较大的值表示对测量值的信任度较低，滤波效果更强但响应更慢
// static int initialized = 0;  // 初始化标志

// double x_pred;
// double p_pred;
// double k;

// double Lowpass_Filter(double x)
// {
//     // double dt;
//     // dt = timestamp - timestamp_previous;

//     // 第一次调用时，初始化状态
//     if (!initialized) {
//         x_est = x;
//         initialized = 1;
//         return x;
//     }

//     x_pred = x_est;
//     p_pred = p_est + q;

//     k = p_pred / (p_pred + r);
//     x_est = x_pred + k * (x - x_pred);
//     p_est = (1 - k) * p_pred;

//     return x_est;
// }

// 一阶低通滤波器参数
static double last_output[3] = {0.0,0.0,0.0};     // 上一次的输出值
static int lp_initialized = 0;     // 初始化标志
static double default_alpha = 0.5; // 默认滤波系数 (0.0 < alpha < 1.0)

// 滑动平均滤波器参数
#define MAX_CHANNELS 5                                // 最多支持的通道数
#define MAX_WINDOW_SIZE 20                            // 最大窗口大小
static double history[MAX_CHANNELS][MAX_WINDOW_SIZE]; // 历史数据
static int index[MAX_CHANNELS] = {0};                 // 每个通道的当前索引
static int initialized[MAX_CHANNELS] = {0};           // 每个通道是否已初始化

/**
 * 简单的一阶低通滤波器
 * 使用默认系数，适用于一般场景
 *
 * x: 新的输入值
 * 返回: 滤波后的值
 */
double Lowpass_Filter(double x,int channel)
{

    // 一阶低通滤波公式: y[n] = alpha * x[n] + (1 - alpha) * y[n-1]
    // alpha值越小，滤波效果越强，但响应越慢
    last_output[channel] = default_alpha * x + (1.0 - default_alpha) * last_output[channel];
    return last_output[channel];
}

/**
 * 可配置的指数低通滤波器
 *
 * new_value: 新的输入值
 * alpha: 滤波系数 (0.0 < alpha < 1.0)
 * 返回: 滤波后的值
 */
// double Exp_Lowpass_Filter(double new_value, double alpha)
// {
//     // 检查alpha值是否在有效范围内
//     if (alpha <= 0.0)
//         alpha = 0.01;
//     if (alpha > 1.0)
//         alpha = 1.0;

//     // 第一次调用时，直接返回输入值
//     if (!lp_initialized)
//     {
//         last_output = new_value;
//         lp_initialized = 1;
//         return new_value;
//     }

//     // 指数低通滤波公式
//     last_output = alpha * new_value + (1.0 - alpha) * last_output;

//     return last_output;
// }

// /**
//  * 设置默认的滤波系数
//  *
//  * alpha: 滤波系数 (0.0 < alpha < 1.0)
//  */
// void Set_Filter_Alpha(double alpha)
// {
//     if (alpha <= 0.0)
//         alpha = 0.01;
//     if (alpha > 1.0)
//         alpha = 1.0;

//     default_alpha = alpha;
// }

// /**
//  * 重置所有滤波器状态
//  */
// void Reset_Filters(void)
// {
//     // 重置一阶滤波器
//     lp_initialized = 0;
//     last_output = 0;

//     // 重置滑动平均滤波器
//     for (int i = 0; i < MAX_CHANNELS; i++)
//     {
//         initialized[i] = 0;
//         index[i] = 0;
//         for (int j = 0; j < MAX_WINDOW_SIZE; j++)
//         {
//             history[i][j] = 0;
//         }
//     }
// }

// /**
//  * 滑动平均低通滤波器
//  *
//  * new_value: 新的输入值
//  * channel: 通道号（0 ~ MAX_CHANNELS-1）
//  * window_size: 滑动窗口大小（1 ~ MAX_WINDOW_SIZE）
//  * 返回: 滤波后的值
//  */
// double MA_Lowpass_Filter(double new_value, int channel, int window_size)
// {
//     double sum = 0;
//     int i, count;

//     // 参数检查
//     if (channel < 0 || channel >= MAX_CHANNELS)
//         return new_value;
//     if (window_size < 1)
//         window_size = 1;
//     if (window_size > MAX_WINDOW_SIZE)
//         window_size = MAX_WINDOW_SIZE;

//     // 存储新值
//     history[channel][index[channel]] = new_value;
//     index[channel] = (index[channel] + 1) % window_size;

//     // 如果是第一次调用，初始化所有窗口值为当前值
//     if (!initialized[channel])
//     {
//         for (i = 0; i < window_size; i++)
//         {
//             history[channel][i] = new_value;
//         }
//         initialized[channel] = 1;
//         return new_value;
//     }

//     // 计算均值
//     for (i = 0, count = 0; count < window_size; count++)
//     {
//         sum += history[channel][(i + count) % window_size];
//     }

//     return sum / window_size;
// }
