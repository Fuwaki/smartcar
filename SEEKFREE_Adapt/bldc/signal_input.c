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

#include "board.h"
#include "bldc_config.h"
#include "motor_control.h"
#include "pwm_out.h"
#include "signal_input.h"

#define PWMIN_PIN   P00

pwmin_struct pwmin;
/*
����ļ�������������ź�֮�󣬰Ѷ��������Ŵ�Сתд�뵽motor�ṹ��
*/
 
uint8 pwm_input_count = 0;
//-------------------------------------------------------------------------------------------------------------------
//  @brief      PWMB���벶���ж�
//  @param      void                        
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void pwmb_isr()interrupt 27
{
    uint16 temp;
	if(PWMB_SR1 & 0x02)
	{
		pwmin.period = (PWMB_CCR1H << 8) + PWMB_CCR1L;	    // CC1�������ڿ��
		PWMB_SR1 = 0;
        
        // ��������PWM�źŵ�Ƶ��
        pwmin.frequency = sys_clk / (PWMB_PSCRL + 1) / pwmin.period;
	}
	
	if(PWMB_SR1 & 0x04)
	{
		pwmin.high_value = (PWMB_CCR2H << 8) + PWMB_CCR2L;   // CC2����ߵ�ƽ���
		PWMB_SR1 = 0;
        
        // Ƶ���ں���ķ�Χ�ڲż���
        if((30 < pwmin.frequency) && (400 > pwmin.frequency))
        {
            // ����ߵ�ƽʱ�� ���ڸߵ�ƽʱ��Ϊ1-2ms����Ч 
            pwmin.high_time = pwmin.high_value;
            
            if((3000 < pwmin.high_time) || (1000 > pwmin.high_time))
            {
                // �ߵ�ƽʱ��������߹��̣�����������Ϊ0
                pwmin.throttle = 0;
            }
            else
            {
                if(2000 < pwmin.high_time)
                {
                    pwmin.high_time = 2000;
                }
                
                // �������Ŵ�С
                temp = pwmin.high_time - 1000;
                // �����������Ŵ�С С������ռ�ձ�����������Ϊ0
                if(temp < ((uint32)1000 * BLDC_MIN_DUTY / BLDC_MAX_DUTY))
                {
                    temp = 0;
                }
                pwmin.throttle = temp;
            }
        }
        
        // ����ռ�ձ�
        motor.duty = (uint32)pwmin.throttle * BLDC_MAX_DUTY / 1000;
	}
    
    if(PWMB_SR1 & 0x01)
    {
        PWMB_SR1 = 0;
				
		pwmin.throttle = 0;
		motor.duty = 0;
		
        // δ��⵽�����ź���������Ŷ�����
		if(motor.duty > 0)
		{
			if(++pwm_input_count >= 2)
			{
				pwm_input_count = 0;
				
				pwmin.throttle = 0;
				motor.duty = 0;
			}
		}
    }
}


//-------------------------------------------------------------------------------------------------------------------
//  @brief      PWMB���벶���ʼ��
//  @param      void                        
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void pwm_input_init(void)
{
    PWMB_PS = 0x0A;		// ͨ�������л�
    PWMB_CCMR1 = 0x01;	// CC5Ϊ����ģʽ,��ӳ�䵽TI5FP5��
	PWMB_CCMR2 = 0x02;	// CC6Ϊ����ģʽ,��ӳ�䵽TI5FP6��
    
	// CC5E �������벶��
	// CC5P ��������TI5F��������
	// CC6E �������벶��
	// CC6P ��������TI5F���½���
    PWMB_CCER1 = 0x31;
    
    PWMB_PSCRH = 0;		// ��Ƶֵ
	PWMB_PSCRL = sys_clk / 1000000 - 1;    // ��Ƶֵ
    PWMB_SMCR = 0x54;	// TS=TI1FP1,SMS=TI1�����ظ�λģʽ
	PWMB_CR1 = 0x01;	// ����PWMB�����ϼ���
	PWMB_IER = 0x07;	// ʹ��CC1��CC2��UIE�ж�

    pwmin.period = 0;
    pwmin.high_value = 0;
    pwmin.high_time = 0;
}