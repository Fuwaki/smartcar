/*********************************************************************************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2020,��ɿƼ�
 * All rights reserved.
 * ��������QQȺ��һȺ��179029047(����)  ��Ⱥ��244861897(����)  ��Ⱥ��824575535
 *
 * �����������ݰ�Ȩ������ɿƼ����У�δ��������������ҵ��;��
 * ��ӭ��λʹ�ò������������޸�����ʱ���뱣����ɿƼ��İ�Ȩ������
 *
 * @file       		pit_timer
 * @company	   		�ɶ���ɿƼ����޹�˾
 * @author     		��ɿƼ�(QQ790875685)
 * @version    		�鿴doc��version�ļ� �汾˵��
 * @Software 		MDK FOR C251 V5.60
 * @Target core		STC32G12K128
 * @Taobao   		https://seekfree.taobao.com/
 * @date       		2024-01-22
 ********************************************************************************************************************/

#include "zf_adc.h"
#include "bldc_config.h"
#include "battery.h"
#include "zf_gpio.h"

#define	ADC_CH		1						/* 1~16, ADCת��ͨ����, ��ͬ���޸� DMA_ADC_CHSW ת��ͨ�� */
#define	ADC_DATA	6						/* 6~n, ÿ��ͨ��ADCת����������, 2*ת������+4, ��ͬ���޸� DMA_ADC_CFG2 ת������ */
#define	DMA_ADDR	0x800					/* DMA���ݴ�ŵ�ַ */
static uint8 xdata adc_dma_buff[ADC_CH][ADC_DATA] _at_ DMA_ADDR;

uint16 battery_voltage;

//-------------------------------------------------------------------------------------------------------------------
//  @brief      ��ص�ѹ��ȡ
//  @param      void                        
//  @return     uint8 	0-���� 1-��ص�ѹ����          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
uint8 battery_voltage_get(void)
{
    uint16 pin_voltage;
    static uint32 low_power_num = 0;
    uint16 adc_reg_value;
	
    adc_reg_value = adc_dma_buff[0][0];
    pin_voltage = (uint32)adc_reg_value * 5000 / 256;       // ��ADCֵת��Ϊʵ�ʵĵ�ѹ
    battery_voltage = (uint32)pin_voltage * 57 / 10;    	// ����Ӳ����ѹ�����ֵ�����ص�ѹ
	
	// ������һ��ADC_DMAת��
	DMA_ADC_CR = 0xc0;
	
    if((BLDC_MIN_BATTERY > battery_voltage))
    {
        low_power_num++;
        if((2000 ) < (low_power_num / 20))		// 2s��������ѹ����ֵ�������ͣ������
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
//  @brief      ��ص�ѹ����ʼ��
//  @param      void                        
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void battery_init(void)
{
	
	gpio_mode(P0_3, GPI_IMPEDANCE);
	
	ADC_CONTR = 0x80;						// ADCʹ��
	ADCTIM = 0x3f;  						// ����ͨ��ѡ��ʱ�䡢����ʱ�䡢����ʱ��
	ADCCFG = 0x0F;							// ����룬ADCת��ʱ�����

	DMA_ADC_STA = 0x00;
	DMA_ADC_CFG = 0x0F;		
	DMA_ADC_RXAH = (uint8)(DMA_ADDR >> 8);	//ADCת�����ݴ洢��ַ
	DMA_ADC_RXAL = (uint8)DMA_ADDR;
	DMA_ADC_CFG2 = 0x00;					// ÿ��ͨ��ADCת������:1
	DMA_ADC_CHSW0 = 0;						// 
	DMA_ADC_CHSW1 = 0x1<<3;					// ADCͨ��ʹ�ܼĴ��� ADC3
	DMA_ADC_CR = 0xc0;						// ʹ��ADC_DMA,ADC��ʼת����
	
    // ��ʼ����ʱ���Ȳɼ�һ�ε�ѹ
    battery_voltage_get();
}