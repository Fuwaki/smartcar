#include <STC32G.H>

/**
 * @brief 初始化ADC
 */
void ADC_Init(void)
{
    P_SW2 |= 0x80;                  // 访问XSFR必须先开启
    
    // ADC时钟设置
    ADCTIM = 0x3f;                  // 设置ADC内部时序，ADC采样时间建议设最大值
    
    // 打开ADC电源
    ADC_CONTR = 0x80;               // ADC_POWER = 1
    
    // 配置ADC为左对齐模式
    ADCCFG = 0x20;                  // 结果左对齐 (RESFMT = 1)
    
    // 延时，ADC上电需要一段时间稳定
    unsigned char i = 100;
    while(--i);
}

/**
 * @brief 获取指定通道的ADC值
 * 
 * @param channel ADC通道
 * @return unsigned int ADC转换结果(12位)
 */
unsigned int GetADC(unsigned char channel)
{
    unsigned int ADC_Value;
    
    // 选择通道并启动ADC
    ADC_CONTR = (ADC_CONTR & 0xF0) | channel;
    ADC_CONTR |= 0x40;              // 启动AD转换
    
    // 等待ADC转换完成
    while (!(ADC_CONTR & 0x20));    // 等待ADC_FLAG = 1
    
    // 清除ADC标志
    ADC_CONTR &= ~0x20;             // ADC_FLAG = 0
    
    // 获取ADC结果
    ADC_Value = ADC_RES;            // 获取高8位结果
    ADC_Value = (ADC_Value << 4) | (ADC_RESL & 0x0F); // 获取完整12位结果
    
    return ADC_Value;
}

/**
 * @brief 获取U相电流值
 * 
 * @return unsigned int U相ADC转换结果
 */
unsigned int Get_U_Current(void)
{
    return GetADC(9);               // P1.1对应通道9
}

/**
 * @brief 获取V相电流值
 * 
 * @return unsigned int V相ADC转换结果
 */
unsigned int Get_V_Current(void)
{
    return GetADC(8);               // P1.0对应通道8
}

/**
 * @brief 获取W相电流值
 * 
 * @return unsigned int W相ADC转换结果
 */
unsigned int Get_W_Current(void)
{
    return GetADC(6);               // P0.6对应通道6
}
