#ifndef __ADC_H__
#define __ADC_H__

/**
 * @brief 初始化ADC
 */
void ADC_Init(void);

/**
 * @brief 获取指定通道的ADC值
 * 
 * @param channel ADC通道
 * @return unsigned int ADC转换结果(12位)
 */
unsigned int GetADC(unsigned char channel);

/**
 * @brief 获取U相电流值
 * 
 * @return unsigned int U相ADC转换结果
 */
unsigned int Get_U_Current(void);

/**
 * @brief 获取V相电流值
 * 
 * @return unsigned int V相ADC转换结果
 */
unsigned int Get_V_Current(void);

/**
 * @brief 获取W相电流值
 * 
 * @return unsigned int W相ADC转换结果
 */
unsigned int Get_W_Current(void);

#endif /* __ADC_H__ */
