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

#ifndef _pwm_out_h_
#define _pwm_out_h_


#include "common.h"

//����ط������ֶ����� ����Ӧ�ò�Ӧ�ö�Ӧ��pwmģ���N����P
//������ʵ�ʵ������� 
//PWM2
#define PWM_A_H_PIN     P13
#define PWM_A_L_PIN     P54

//PWM3
#define PWM_B_H_PIN     P15
#define PWM_B_L_PIN     P14
//PWN4
#define PWM_C_H_PIN     P17
#define PWM_C_L_PIN     P16



extern uint8 g_use_complementary;

void pwm_brake(void);
void pwm_close_output(void);
void pwm_a_bn_output(uint8 use_comp);
void pwm_a_cn_output(uint8 use_comp);
void pwm_b_cn_output(uint8 use_comp);
void pwm_b_an_output(uint8 use_comp);
void pwm_c_an_output(uint8 use_comp);
void pwm_c_bn_output(uint8 use_comp);
void pwm_isr_close(void);
void pwm_isr_open(void);
void pwm_out_duty_update(uint16 duty);
void pwm_out_init(void);

#endif
