// 卡尔曼滤波器参数
static double x_est = 0;     // 状态估计值
static double p_est = 1;     // 估计误差协方差
static double q = 0.01;      // 过程噪声协方差 较大的值使滤波器更快响应变化，但会增加输出噪声
static double r = 0.1;       // 测量噪声协方差 较大的值表示对测量值的信任度较低，滤波效果更强但响应更慢
static int initialized = 0;  // 初始化标志

double Lowpass_Filter(double x)
{
    // 第一次调用时，初始化状态
    if (!initialized) {
        x_est = x;
        initialized = 1;
        return x;
    }

    double x_pred = x_est;
    double p_pred = p_est + q;
    
    double k = p_pred / (p_pred + r);
    x_est = x_pred + k * (x - x_pred);
    p_est = (1 - k) * p_pred;
    
    return x_est;
}