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

#include "intrins.h"
#include "zf_gpio.h"
#include "bldc_config.h"
#include "comparator.h"
#include "pwm_out.h"

uint8 g_use_complementary = 0;

//-------------------------------------------------------------------------------------------------------------------
//  @brief      ����ʱ
//  @param      void
//  @return     void
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void delay_500ns(void)
{
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      ɲ�����ر�����������������ţ�
//  @param      void
//  @return     void
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void pwm_brake(void)
{
	PWMA_ENO = 0;
	PWM_A_L_PIN = 1;
	PWM_B_L_PIN = 1;
	PWM_C_L_PIN = 1;
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      �ر����
//  @param      void
//  @return     void
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void pwm_close_output(void)
{
	PWM_A_H_PIN = 0;
	PWM_B_H_PIN = 0;
	PWM_C_H_PIN = 0;

	PWM_A_L_PIN = 0;
	PWM_B_L_PIN = 0;
	PWM_C_L_PIN = 0;
	// PWMA_ENO = 0;
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      ����A��B��
//  @param      void
//  @return     void
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
// A��ӦPWM2
void pwm_a_bn_output(uint8 use_comp)

{
	pwm_close_output();
	if (use_comp)
	{
		// ����PWM���
		PWMA_CCER1 = 0x0F << 4;
		PWMA_CCER2 = 0x00;
		PWMA_ENO = (3 << 2);
	}
	else
	{
		PWMA_CCER1 = 0x30;
		PWMA_CCER2 = 0x00;
		PWMA_ENO = 1 << 2;
	}
	PWM_B_L_PIN = 1;
	comparator_select_c();
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      ����A��C��
//  @param      void
//  @return     void
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void pwm_a_cn_output(uint8 use_comp)
{
	pwm_close_output();
	if (use_comp)
	{
		PWMA_CCER1 = 0x0F << 4;
		PWMA_CCER2 = 0x00;
		PWMA_ENO = (3 << 2);
	}
	else
	{
		PWMA_CCER1 = 0x30;
		PWMA_CCER2 = 0x00;
		PWMA_ENO = 1 << 2;
	}
	PWM_C_L_PIN = 1;
	comparator_select_b();
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      ����B��C��
//  @param      void
//  @return     void
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
// Bͨ����PWM3
void pwm_b_cn_output(uint8 use_comp)
{
	pwm_close_output();
	if (use_comp)
	{
		PWMA_CCER1 = 0x00;
		PWMA_CCER2 = 0x0F << 0;
		PWMA_ENO = (3 << 4);
	}
	else
	{
		PWMA_CCER1 = 0x00;
		PWMA_CCER2 = 0x03;
		PWMA_ENO = 1 << 4;
	}
	PWM_C_L_PIN = 1;
	comparator_select_a();
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      ����B��A��
//  @param      void
//  @return     void
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------
void pwm_b_an_output(uint8 use_comp)
{
	pwm_close_output();
	if (use_comp)
	{
		PWMA_CCER1 = 0x00;
		PWMA_CCER2 = 0x0F << 0;
		PWMA_ENO = (3 << 4);
	}
	else
	{
		PWMA_CCER1 = 0x00;
		PWMA_CCER2 = 0x03;
		PWMA_ENO = 1 << 4;
	}
	PWM_A_L_PIN = 1;
	comparator_select_c();
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      ����C��A��
//  @param      void
//  @return     void
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
// Cͨ����PWM4
void pwm_c_an_output(uint8 use_comp)
{
	pwm_close_output();
	if (use_comp)
	{
		PWMA_CCER1 = 0x00;
		PWMA_CCER2 = 0x0F << 4;
		PWMA_ENO = (3 << 6);
	}
	else
	{
		PWMA_CCER1 = 0x00;
		PWMA_CCER2 = 0x30;
		PWMA_ENO = 1 << 6;
	}
	PWM_A_L_PIN = 1;
	comparator_select_b();
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      ����C��B��
//  @param      void
//  @return     void
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void pwm_c_bn_output(uint8 use_comp)
{
	pwm_close_output();
	if (use_comp)
	{
		PWMA_CCER1 = 0x00;
		PWMA_CCER2 = 0x0F << 4;
		PWMA_ENO = (3 << 6);
	}
	else
	{
		PWMA_CCER1 = 0x00;
		PWMA_CCER2 = 0x30;
		PWMA_ENO = 1 << 6;
	}
	PWM_B_L_PIN = 1;
	comparator_select_a();
}

////-------------------------------------------------------------------------------------------------------------------
////  @brief      �ر�PWM�ж�
////  @param      void
////  @return     void
////  @since      v1.0
////  Sample usage:
////-------------------------------------------------------------------------------------------------------------------
// void pwm_isr_close(void)
//{
//     PWMA_IER = 0;
// }

////-------------------------------------------------------------------------------------------------------------------
////  @brief      ����PWM�ж�
////  @param      void
////  @return     void
////  @since      v1.0
////  Sample usage:
////-------------------------------------------------------------------------------------------------------------------
// void pwm_isr_open(void)
//{
//     PWMA_IER = 0x10;
// }

//-------------------------------------------------------------------------------------------------------------------
//  @brief      ����PWMռ�ձ�
//  @param      void
//  @return     void
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void pwm_out_duty_update(uint16 duty)
{
	uint8 temp_h, temp_l;

	temp_h = (duty >> 8) & 0xFF;
	temp_l = (uint8)duty;
	// �ֽڲ�� ����д��

	PWMA_CCR1H = temp_h;
	PWMA_CCR1L = temp_l;

	PWMA_CCR2H = temp_h;
	PWMA_CCR2L = temp_l;

	PWMA_CCR3H = temp_h;
	PWMA_CCR3L = temp_l;
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      PWM��ʼ�������Ķ��뷽ʽ��
//  @param      void
//  @return     void
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void pwm_out_init(void)
{
	PWM_A_H_PIN = 0;
	PWM_A_L_PIN = 0;
	PWM_B_H_PIN = 0;
	PWM_B_L_PIN = 0;
	PWM_C_H_PIN = 0;
	PWM_C_L_PIN = 0;

	// ��ʼ���ͱ�Ϊ�������
	gpio_mode(P5_4, GPO_PP);
	gpio_mode(P1_3, GPO_PP);
	gpio_mode(P1_7, GPO_PP);
	gpio_mode(P1_6, GPO_PP);
	gpio_mode(P1_5, GPO_PP);
	gpio_mode(P1_4, GPO_PP);

	PWMA_CCER1 = 0;
	PWMA_CCER2 = 0;
	PWMA_SR1 = 0;
	PWMA_SR2 = 0;
	PWMA_ENO = 0;
	PWMA_IER = 0;

	// ����PWM����
	// Pin Switch
	PWMA_PS = 0x00; // 00000000  ����pwm
	// Capture Compare Mode Register
	// ͨ��B A��
	PWMA_CCMR2 = 0x78; // ͨ��ģʽ����, PWMģʽ2, Ԥװ������
	// CCMR pwmģʽ
	// Capture Compare Register
	PWMA_CCR2H = 0;
	PWMA_CCR2L = 0;
	// Capture Compare Enable Register
	PWMA_CCER1 |= 0x30; // �����Ƚ����, �͵�ƽ��Ч
	// ͨ��C B��
	// FIXME ������ǲ��Ǹ���� ��������PWMģʽ1��
	PWMA_CCMR3 = 0x78; // ͨ��ģʽ����, PWMģʽ1, Ԥװ������
	PWMA_CCR3H = 0;
	PWMA_CCR3L = 0;
	PWMA_CCER2 |= 0x03; // �����Ƚ����, �͵�ƽ��Ч
	// ͨ��D C��
	PWMA_CCMR4 = 0x78; // ͨ��ģʽ����, PWMģʽ1, Ԥװ������
	PWMA_CCR4H = 0;
	PWMA_CCR4L = 0;
	PWMA_CCER2 |= 0x30; // �����Ƚ����, �͵�ƽ��Ч

	//    PWMA_IER    = 0x10;     // ����������
	// PWMA_ENO = 0xff;//0X15;

	// Ԥ��Ƶ
	PWMA_PSCRH = 0;
	PWMA_PSCRL = 0;

	// ��������
	PWMA_ARRH = (uint8)((BLDC_MAX_DUTY - 1) >> 8);
	PWMA_ARRL = (uint8)((BLDC_MAX_DUTY - 1) & 0xff);

	PWMA_BKR = 0x80; // �����ʹ�� �൱���ܿ���
	PWMA_CR1 = 0x85; // ʹ�ܼ�����, �����Զ���װ�ؼĴ�������, ���ض���, ���ϼ���, ֻ�м�������������Ŵ��������ж�,  bit7=1:д�Զ���װ�ؼĴ�������(�����ڲ��ᱻ����), =0:ֱ��д�Զ���װ�ؼĴ�����(���ڿ��ܻ��ҵ�)
	PWMA_EGR = 0x01; // ����һ�θ����¼�, ��������������Ƶ������, װ��Ԥ��Ƶ�Ĵ�����ֵ

	// �տ�ʼʹ��ɲ����
	PWM_A_L_PIN = 1;
	PWM_B_L_PIN = 1;
	PWM_C_L_PIN = 1;
}
