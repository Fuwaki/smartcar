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


#ifndef _signal_input_h
#define _signal_input_h



#include "common.h"


typedef struct
{
    uint16 frequency;   // �ź�Ƶ��
    uint16 period;      // �ź�����
    uint16 high_value;  // �źŸߵ�ƽ����ֵ
    uint16 high_time;   // �źŸߵ�ƽʱ�� us
    uint16 throttle;    // ������Ŵ�С
}pwmin_struct;


extern pwmin_struct pwmin;

void pwm_input_init(void);

#endif
