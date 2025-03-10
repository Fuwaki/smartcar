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

#include "zf_tim.h"
#include "zf_gpio.h"
#include "zf_delay.h"
#include "bldc_config.h"
#include "pwm_out.h"
#include "signal_input.h"
#include "motor_control.h"
#include "battery.h"
#include "pit_timer.h"
#include "comparator.h"
#include "motor_control.h"
#include "board.h"
#include "sine_control.h"


#pragma warning disable = 183


#define LED_PIN P42

uint32 pit_count = 0;

/*
�������߼�
*/
//-------------------------------------------------------------------------------------------------------------------
//  @brief      LED�ƹ����
//  @param      void
//  @return     void
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void led_control(void)
{
    // LED״̬��ʾ
    if(BYTE_LOW_VOLTAGE == motor.run_flag)
    {
        // ��ص�ѹ���ͣ�LED���� ����
        if(0 == (pit_count%10000))
        {
            LED_PIN = 0;
        }
        else if(1000 == (pit_count%10000))
        {
            LED_PIN = 1;
        }
    }
    else if(motor.restart_delay)
    {
        // ���ڶ�ת����ֹͣ LED����
        if(0 == (pit_count%500))
        {
            LED_PIN = !LED_PIN;
        }
    }
    else if(MOTOR_IDLE == motor.run_flag)
    {
        // ֹͣ LED����
        if(0 == (pit_count%10000))
        {
            LED_PIN = !LED_PIN;
        }
    }
    else if(MOTOR_START == motor.run_flag)
    {
        // �������� LED������
        if(0 == (pit_count%5000))
        {
            LED_PIN = !LED_PIN;
        }
    }
    else if(MOTOR_OPEN_LOOP == motor.run_flag)
    {
        // ���������ά��һ��ʱ�� �Ͽ���˸
        if(0 == (pit_count%1000))
        {
            LED_PIN = !LED_PIN;
        }
    }
    else if(MOTOR_CLOSE_LOOP == motor.run_flag)
    {
        // �������� LED����
        LED_PIN = 0;
    }
}

void pit_motor_control()
{

    static uint8 gpio_state = 0;
    static uint16 stall_time_out_check = 0;
	
    uint8 pin_state = 0;
	static uint8 oled_pin_state = 0;
	
    switch(motor.run_flag)
    {
		case MOTOR_START:
		{
			uint16 sine_delay;
			ET0 = 0; 					// �رն�ʱ��0�ж�
			IE2 = ~0x40;			  	// �رն�ʱ��4�ж�
			comparator_close_isr();		// �رձȽ����ж�
			stall_time_out_check = 0;	// ��ת������������Ϊ0
			motor.commutation_num = 0;	// ����������������Ϊ0
			
			#if BLDC_USE_SINE_START

			sine_init();
			do
			{
				sine_start(BLDC_SINE_START_DUTY);
				
				T4T3M &= ~0x80; // ֹͣ��ʱ��
				T4L = 0;
				T4H = 0;
				T4T3M |= 0x80;  // ������ʱ��
				
				sine_delay = 420 / BLDC_POLES;
				while(((T4H << 8) | T4L) < sine_delay);
			}while(motor_a_position != 0);
			
			motor.step = 2;
			motor.commutation_time[0] = 6000;
			motor.commutation_time[1] = 6000;
			motor.commutation_time[2] = 6000;
			motor.commutation_time[3] = 6000;
			motor.commutation_time[4] = 6000;
			motor.commutation_time[5] = 6000;
			motor.commutation_time_sum = motor.commutation_time[0] + motor.commutation_time[1] + motor.commutation_time[2] + \
										 motor.commutation_time[3] + motor.commutation_time[4] + motor.commutation_time[5] ;
			motor.filter_commutation_time_sum = motor.commutation_time_sum;
			// ��ն�ʱ��������ֵ
			T4T3M &= ~0x80; // ֹͣ��ʱ��
			T4L = 0;
			T4H = 0;
			T4T3M |= 0x80;  // ������ʱ��
			
			IE2 &= ~0x20; 	// �رն�ʱ��3�ж�
			

			motor.run_flag = MOTOR_OPEN_LOOP;

			// ����
			motor_next_step();
			motor_commutation();
			 
			
			// ����ռ�ձ�
			motor.duty_register = motor.duty;	
			if(motor.commutation_num < (BLDC_CLOSE_LOOP_WAIT))
            {
                 if (motor.duty_register < BLDC_PWM_DEADTIME)
                 {
                     motor.duty_register = BLDC_PWM_DEADTIME;
                 }
                 if (motor.duty_register > (BLDC_MAX_DUTY / 10 + BLDC_PWM_DEADTIME))
                 {
                     motor.duty_register = BLDC_MAX_DUTY / 10 + BLDC_PWM_DEADTIME;
                 }
            }
			pwm_out_duty_update(motor.duty_register);
			
			#else

			// ��������ռ�ձ�
			motor.duty_register = motor.duty;//((float)BLDC_START_VOLTAGE * BLDC_MAX_DUTY / ((float)battery_voltage/1000));
			pwm_out_duty_update(motor.duty_register);
			if(((T4H << 8) | T4L) > 20*1000)
			{
				motor.commutation_time[0] = 6000;
				motor.commutation_time[1] = 6000;
				motor.commutation_time[2] = 6000;
				motor.commutation_time[3] = 6000;
				motor.commutation_time[4] = 6000;
				motor.commutation_time[5] = 6000;
				motor.commutation_time_sum = motor.commutation_time[0] + motor.commutation_time[1] + motor.commutation_time[2] + \
											 motor.commutation_time[3] + motor.commutation_time[4] + motor.commutation_time[5] ;
				motor.filter_commutation_time_sum = motor.commutation_time_sum;
				// ��ն�ʱ��������ֵ
				T4T3M &= ~0x80; // ֹͣ��ʱ��
				T4L = 0;
				T4H = 0;
				T4T3M |= 0x80;  // ������ʱ��
				
				IE2 &= ~0x20; 	// �رն�ʱ��3�ж�
				
				motor_next_step();
				motor_commutation();
				motor.run_flag = MOTOR_OPEN_LOOP;
			}
			#endif
		}
		break;
		case MOTOR_OPEN_LOOP:
		{
			uint8 motor_next_step_flag = 0;
			uint16 temp_commutation_time = (T4H << 8) | T4L;
			// ͨ��λ�Ƶķ�ʽ������ƽ���ݴ��������
			gpio_state = (gpio_state << 1) | (CMPCR1 & 0x01);
			if (!(motor.step % 2))
			{
				//�½���
				if(gpio_state == 0)
				{
					motor_next_step_flag = 1;
//					gpio_state = 0x0F;
				}
			}
			else
			{
				// ������
				if(gpio_state == 0xFF)
				{
					motor_next_step_flag = 1;
//					gpio_state = 0xF0;
				}
			}
			
			if(motor_next_step_flag)
			{
				T4T3M &= ~0x80; // ֹͣ��ʱ��
				T4L = 0;
				T4H = 0;
				T4T3M |= 0x80;  // ������ʱ��
				// ��ת�������
				stall_time_out_check = 0;
				// ȥ�����������
				motor.commutation_time_sum -= motor.commutation_time[motor.step];
				// ���滻��ʱ��
				motor.commutation_time[motor.step] = temp_commutation_time;
				// �����µĻ���ʱ�䣬��6�λ�����ʱ��
				motor.commutation_time_sum += motor.commutation_time[motor.step];
				// �����������һ
				motor.commutation_num++;
				// һ�׵�ͨ�˲�
				motor.filter_commutation_time_sum = (motor.filter_commutation_time_sum * 3 + motor.commutation_time_sum * 1) >> 2;
				// �ȴ�15��
				while(((T4H << 8) | T4L) < (motor.filter_commutation_time_sum / 24));
				// ���step��һ
				motor_next_step();
				// �������
				motor_commutation();
			}
			else
			{
				// ��ת���
				if((10 * 200) < stall_time_out_check++)
				{
					stall_time_out_check = 0;
					motor.commutation_num = 0;
					motor.run_flag = MOTOR_STOP_STALL;
				}
			}
			
			// �ջ�״̬�л�
			//if((motor.commutation_num > (BLDC_OPEN_LOOP_WAIT)) && (motor.filter_commutation_time_sum < 30000))
			if(motor.commutation_num > BLDC_OPEN_LOOP_WAIT)
			{
				// ��ռ�����
				motor.commutation_failed_num = 0;
				// ���һЩ����
				motor.commutation_num = 0;
				stall_time_out_check = 0;
				// �򿪱Ƚ����ж�
				comparator_open_isr();
				// ʹ�ܶ�ʱ��4�ж�
				IE2 |= 0x40;
				// ʹ�ܶ�ʱ��4
				T4T3M |= 0x80;
				// �л�Ϊ�ջ�״̬
				motor.run_flag = MOTOR_CLOSE_LOOP;
			}
			
			// ����20msû�м�⵽���䣬��ӽ�������ʼ״̬
			if(((T4H << 8) | T4L) > 20*1000)
			{
				T4T3M &= ~0x80; // ֹͣ��ʱ��
				T4L = 0;
				T4H = 0;
				T4T3M |= 0x80;  // ������ʱ��
				// ���step��һ
				motor_next_step();
				// �������
				motor_commutation();
				motor.run_flag = MOTOR_START;
			}
		}
		break;
		case MOTOR_CLOSE_LOOP:
		{
			pin_state = (CMPCR1 & 0x01);
			// ͨ��PA2���ŵĵ�ƽ״̬�����ж�ת���
			if(oled_pin_state != pin_state)
			{
				stall_time_out_check = 0;
				oled_pin_state = pin_state;
			}
			else
			{
				// �������һֱ��һ��״̬���ҳ����� BLDC_STALL_TIME_OUT ms ����Ϊ��ת��
				if(stall_time_out_check++ > (10 * 200))          // 10KHZ��0.1ms����һ��
				{
					stall_time_out_check = 0;
					motor.run_flag = MOTOR_STOP_STALL;
				}
			}
			
			// �ջ���������		
			if(motor.commutation_num < (BLDC_CLOSE_LOOP_WAIT))
            {
                 if (motor.duty_register < BLDC_PWM_DEADTIME)
                 {
                     motor.duty_register = BLDC_PWM_DEADTIME;
                 }
                 if (motor.duty_register > (BLDC_MAX_DUTY / 10 + BLDC_PWM_DEADTIME))
                 {
                     motor.duty_register = BLDC_MAX_DUTY / 10 + BLDC_PWM_DEADTIME;
                 }
            }
			

			// �����Ӽ���
			// BLDC_MAX_DUTY = 1458 
			// 50us��һ��PIT�жϣ�һ�����20�Ρ�
			// ���ÿһ�ν��������޸�һ��duty��ֵ����ô72ms���ܴ�0����100
			if((motor.duty_register != motor.duty) && (pit_count % BLDC_SPEED_INCREMENTAL == 0))
			{
				if(motor.duty > motor.duty_register)
				{
					motor.duty_register ++;
					if(BLDC_MAX_DUTY < motor.duty_register)
					{
						motor.duty_register = BLDC_MAX_DUTY;
					}
				}
				else
				{
					motor.duty_register--;
					if (motor.duty_register < BLDC_PWM_DEADTIME)
					{
						motor.duty_register = BLDC_PWM_DEADTIME;
					}
				}
				
//				// ֻ���ڼ��ٵ�ʱ��򿪻���PWM
//				if((int16)((int16)motor.duty_register - (int16)motor.duty) >= (BLDC_MAX_DUTY >> 4))
//				{
//					g_use_complementary = 1;
//				}
//				else
//				{
//					g_use_complementary = 0;
//				}
				
				pwm_out_duty_update(motor.duty_register);
			}
			
	
		}
		break;
		case MOTOR_STOP_STALL:
		{
			motor_stop();
			IE2 = ~0x40;				// �رն�ʱ��4�ж�
			comparator_close_isr();		// �رձȽ����ж�
			motor.run_flag = MOTOR_RESTART;
			motor.restart_delay = BLDC_START_DELAY;
		}
		break;
		case MOTOR_RESTART:
		{
			if(motor.restart_delay)
			{
				// ��ʱ����ʱ�����
				motor.restart_delay--;
			}
			else
			{
				motor.run_flag = MOTOR_IDLE;
			}
		}
		break;
		case MOTOR_IDLE:
		case BYTE_LOW_VOLTAGE:
		{
			ET0 = 0; 					// �رն�ʱ��0�ж�
			IE2 = ~0x40;			  	// �رն�ʱ��4�ж�
			comparator_close_isr();		// �رձȽ����ж�
			motor_stop();
		}
		break;
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      ��ʱ��1�ж�
//  @param      void
//  @return     void
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void TM1_Isr() interrupt 3
{
    pit_count++;
    if(battery_voltage_get())
    {
        motor.run_flag = BYTE_LOW_VOLTAGE;
    }
    // ��ص�ѹ���Ϊ������ȼ�������ѹ���ͣ��Ͳ�ת
    if(motor.run_flag != BYTE_LOW_VOLTAGE)
    {
        // �����ռ�ձ�Ϊ0����Ϊ����״̬
        if(motor.duty == 0)
        {
            motor.run_flag = MOTOR_IDLE;
        }
        else
        {
            // �����ռ�ձȲ�Ϊ0����ӿ���״̬תΪ�����ʼ
            if(motor.run_flag == MOTOR_IDLE)
            {
                motor.run_flag = MOTOR_START;
            }
        }
    }
    led_control();
    pit_motor_control();
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      ���ڶ�ʱ����ʼ��
//  @param      void
//  @return     void
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void pit_timer_init(void)
{
	uint8 freq_div = 0;                
    uint16 period_temp = 0;               
    uint16 temp = 0;
	uint16 tim_us = 50;		// ����Ϊ0.5ms�ж�һ��,200hzƵ��

	AUXR |= 0x40;													// ����Ϊ1Tģʽ
	freq_div = ((tim_us * sys_clk / 1000000) / (1 << 15));          // ����Ԥ��Ƶ
	period_temp = ((tim_us * sys_clk / 1000000) / (freq_div + 1));  // �����Զ���װ��ֵ
	temp = (uint16)65536 - period_temp;

	TM1PS = freq_div;	// ���÷�Ƶֵ
	TMOD |= 0x00; 		// ģʽ 0
	TL1 = temp;
	TH1 = temp >> 8;
	TR1 = 1; 			// ������ʱ��
	ET1 = 1; 			// ʹ�ܶ�ʱ���ж�
	
	pit_count = 0;
    motor.run_flag = MOTOR_IDLE;
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      LED��ʼ��
//  @param      void
//  @return     void
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void led_init(void)
{
    gpio_mode(P4_4, GPO_PP);
    LED_PIN = 0;
}