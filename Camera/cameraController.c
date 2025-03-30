#include <AI8051U.H>
#include <intrins.h>
#include "cameraController.h"

// CCD参数定义
#define CCD_CLK P10      // CCD时钟信号
#define CCD_SI  P11      // CCD起始信号
#define CCD_AO  P12      // CCD模拟输出
#define CCD_LENGTH 128   // CCD像素点数量

// CCD数据缓冲区
unsigned char xdata RawData[CCD_LENGTH];    // 原始采集数据
unsigned char xdata BinaryData[CCD_LENGTH]; // 二值化后数据
unsigned char xdata Threshold = 128;        // 默认阈值

void Delay1us(void)	//@40.000MHz
{
	unsigned long edata i;

	_nop_();
	_nop_();
	_nop_();
	i = 8UL;
	while (i) i--;
}

// CCD初始化函数
void CCD_Init()
{
    // 配置IO口
    P1M0 |= 0x03;  // P10、P11设为推挽输出
    P1M1 &= ~0x03;
    P1M0 &= ~0x04; // P12设为高阻输入
    P1M1 |= 0x04;
    
    // 初始状态
    CCD_CLK = 0;
    CCD_SI = 0;
    
    // 初始化ADC
    P_SW2 |= 0x80;      // 开启EAXFR
    ADCCFG = 0x01;      // ADC配置: 设置为高位优先，结果左对齐
    ADCEXCFG = 0x00;    // 关闭ADC内部温度传感器
    ADC_CONTR = 0x80;   // 只开启ADC电源，不选择通道
    
    // 延时等待CCD稳定
    Delay1us();
}

// CCD采集函数
void CCD_CollectData()
{
    unsigned char i;
    
    // 发送起始信号
    CCD_SI = 1;
    Delay1us();
    CCD_CLK = 1;
    Delay1us();
    
    // 结束起始信号
    CCD_SI = 0;
    Delay1us();
    
    // 采集第一个像素(无效像素)
    CCD_CLK = 0;
    Delay1us();
    CCD_CLK = 1;
    Delay1us();
    
    // 采集有效数据
    for(i = 0; i < CCD_LENGTH; i++)
    {
        CCD_CLK = 0;        // 时钟下降沿
        Delay1us();
        
        // 读取模拟值，需要通过ADC转换
        ADC_CONTR = 0x82;   // 开启ADC电源，选择ADC通道2 (P12)
        
        // 启动ADC转换
        ADC_CONTR |= 0x40;  // 开始AD转换
        NOP10();            // 等待采样电容充电完全
        while (!(ADC_CONTR & 0x20)); // 等待ADC转换完成
        ADC_CONTR &= ~0x20; // 清除标志位
        
        RawData[i] = ADC_RES; // 读取ADC结果
        
        CCD_CLK = 1;        // 时钟上升沿
        Delay1us();
    }
}

// 计算CCD数据的动态阈值
unsigned char CCD_CalculateThreshold()
{
    unsigned int total = 0;
    unsigned char max = 0, min = 255;
    unsigned char i;
    
    // 找出最大值和最小值，并计算总和
    for(i = 0; i < CCD_LENGTH; i++)
    {
        if(RawData[i] > max) max = RawData[i];
        if(RawData[i] < min) min = RawData[i];
        total += RawData[i];
    }
    
    // 计算平均值，然后使用(最大值+最小值+平均值)/3作为阈值
    return (max + min + (total / CCD_LENGTH)) / 3;
}

// CCD数据二值化处理
void CCD_BinarizeData(unsigned char useAutoThreshold)
{
    unsigned char i;
    
    // 是否使用自动计算的阈值
    if(useAutoThreshold)
    {
        Threshold = CCD_CalculateThreshold();
    }
    
    // 二值化处理
    for(i = 0; i < CCD_LENGTH; i++)
    {
        if(RawData[i] > Threshold)
            BinaryData[i] = 1;  // 白线
        else
            BinaryData[i] = 0;  // 黑底
    }
}

// 设置手动阈值
void CCD_SetThreshold(unsigned char threshold)
{
    Threshold = threshold;
}

// 获取当前阈值
unsigned char CCD_GetThreshold()
{
    return Threshold;
}

// 获取二值化数据指针
unsigned char* CCD_GetBinaryData()
{
    return BinaryData;
}

// 获取原始数据指针
unsigned char* CCD_GetRawData()
{
    return RawData;
}

// 完整的CCD采集和处理函数
void CCD_Process(unsigned char useAutoThreshold)
{
    CCD_CollectData();          // 采集数据
    CCD_BinarizeData(useAutoThreshold); // 二值化处理
}

//ccd的难点主要在找赛道吧
//2333 完全没思路
//nild
void StartCCD() //使用这个之后可以直接调用二值化数据  在tracking 里面写函数qwq
{
    CCD_CollectData();
    CCD_BinarizeData(1);
    CCD_GetBinaryData();
}