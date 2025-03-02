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


#ifndef _conparator_h_
#define _conparator_h_

#include "common.h"





extern uint16 motor_commutation_time;



void  comparator_select_a(void);
void  comparator_select_b(void);
void  comparator_select_c(void);
void  comparator_rising(void);
void  comparator_falling(void);
uint8 comparator_result_get(void);
void  comparator_close_isr(void);
void  comparator_open_isr(void);
void  comparator_init(void);


#endif