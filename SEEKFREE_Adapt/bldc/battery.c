/*
电池电压检测，我们不需要在电调里搞
*/
#include "zf_adc.h"
#include "bldc_config.h"
#include "battery.h"
#include "zf_gpio.h"

#define	ADC_CH		1						/* 1~16, ADC转换通道数, 需同步修改 DMA_ADC_CHSW 转换通道 */
#define	ADC_DATA	6						/* 6~n, 每个通道ADC转换数据总数, 2*转换次数+4, 需同步修改 DMA_ADC_CFG2 转换次数 */
#define	DMA_ADDR	0x800					/* DMA数据存放地址 */
static uint8 xdata adc_dma_buff[ADC_CH][ADC_DATA] _at_ DMA_ADDR;

uint16 battery_voltage;

//-------------------------------------------------------------------------------------------------------------------
//  @brief      电池电压获取
//  @param      void                        
//  @return     uint8 	0-正常 1-电池电压过低          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
uint8 battery_voltage_get(void)
{
    uint16 pin_voltage;
    static uint32 low_power_num = 0;
    uint16 adc_reg_value;
	
    adc_reg_value = adc_dma_buff[0][0];
    pin_voltage = (uint32)adc_reg_value * 5000 / 256;       // 将ADC值转换为实际的电压
    battery_voltage = (uint32)pin_voltage * 57 / 10;    	// 根据硬件分压电阻的值计算电池电压
	
	// 开启下一次ADC_DMA转换
	DMA_ADC_CR = 0xc0;
	
    if((BLDC_MIN_BATTERY > battery_voltage))
    {
        low_power_num++;
        if((2000 ) < (low_power_num / 20))		// 2s都超过电压保护值，则进行停机保护
        {
           return 1;   
        }
    }
    else
    {
        low_power_num = 0;
    }

	return 0;
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      电池电压检测初始化
//  @param      void                        
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void battery_init(void)
{
	
	gpio_mode(P0_3, GPI_IMPEDANCE);
	
	ADC_CONTR = 0x80;						// ADC使能
	ADCTIM = 0x3f;  						// 设置通道选择时间、保持时间、采样时间
	ADCCFG = 0x0F;							// 左对齐，ADC转换时间最大

	DMA_ADC_STA = 0x00;
	DMA_ADC_CFG = 0x0F;		
	DMA_ADC_RXAH = (uint8)(DMA_ADDR >> 8);	//ADC转换数据存储地址
	DMA_ADC_RXAL = (uint8)DMA_ADDR;
	DMA_ADC_CFG2 = 0x00;					// 每个通道ADC转换次数:1
	DMA_ADC_CHSW0 = 0;						// 
	DMA_ADC_CHSW1 = 0x1<<3;					// ADC通道使能寄存器 ADC3
	DMA_ADC_CR = 0xc0;						// 使能ADC_DMA,ADC开始转换。
	
    // 初始化的时候先采集一次电压
    battery_voltage_get();
}