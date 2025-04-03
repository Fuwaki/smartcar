/*********************************************************************************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2020,逐飞科技
 * All rights reserved.
 * 技术讨论QQ群：一群：179029047(已满)  二群：244861897(已满)  三群：824575535
 *
 * 以下所有内容版权均属逐飞科技所有，未经允许不得用于商业用途，
 * 欢迎各位使用并传播本程序，修改内容时必须保留逐飞科技的版权声明。
 *
 * @file       		pit_timer
 * @company	   		成都逐飞科技有限公司
 * @author     		逐飞科技(QQ790875685)
 * @version    		查看doc内version文件 版本说明
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
这个文件捕获了输入的信号之后，把读出的油门大小转写入到motor结构体
*/
 
uint8 pwm_input_count = 0;
//-------------------------------------------------------------------------------------------------------------------
//  @brief      PWMB输入捕获中断
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
		pwmin.period = (PWMB_CCR1H << 8) + PWMB_CCR1L;	    // CC1捕获周期宽度
		PWMB_SR1 = 0;
        
        // 计算输入PWM信号的频率
        pwmin.frequency = sys_clk / (PWMB_PSCRL + 1) / pwmin.period;
	}
	
	if(PWMB_SR1 & 0x04)
	{
		pwmin.high_value = (PWMB_CCR2H << 8) + PWMB_CCR2L;   // CC2捕获高电平宽度
		PWMB_SR1 = 0;
        
        // 频率在合理的范围内才计算
        if((30 < pwmin.frequency) && (400 > pwmin.frequency))
        {
            // 计算高电平时间 仅在高电平时间为1-2ms内有效 
            pwmin.high_time = pwmin.high_value;
            
            if((3000 < pwmin.high_time) || (1000 > pwmin.high_time))
            {
                // 高电平时间过长或者过短，则油门设置为0
                pwmin.throttle = 0;
            }
            else
            {
                if(2000 < pwmin.high_time)
                {
                    pwmin.high_time = 2000;
                }
                
                // 计算油门大小
                temp = pwmin.high_time - 1000;
                // 如果输入的油门大小 小于启动占空比则油门设置为0
                if(temp < ((uint32)1000 * BLDC_MIN_DUTY / BLDC_MAX_DUTY))
                {
                    temp = 0;
                }
                pwmin.throttle = temp;
            }
        }
        
        // 更新占空比
        motor.duty = (uint32)pwmin.throttle * BLDC_MAX_DUTY / 1000;
	}
    
    if(PWMB_SR1 & 0x01)
    {
        PWMB_SR1 = 0;
				
		pwmin.throttle = 0;
		motor.duty = 0;
		
        // 未检测到输入信号则输出油门都清零
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
    motor.duty = 600;
    motor.duty = 400;
}


//-------------------------------------------------------------------------------------------------------------------
//  @brief      PWMB输入捕获初始化
//  @param      void                        
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void pwm_input_init(void)
{
    PWMB_PS = 0x00;		// 通道引脚切换
    PWMB_CCMR1 = 0x01;	// CC5为输入模式,且映射到TI5FP5上
	PWMB_CCMR2 = 0x02;	// CC6为输入模式,且映射到TI5FP6上
    // TODO CC5和CC6还没有处理好
	// CC5E 开启输入捕获
	// CC5P 捕获发生在TI5F的上升沿
	// CC6E 开启输入捕获
	// CC6P 捕获发生在TI5F的下降沿
    PWMB_CCER1 = 0x31;
    
    PWMB_PSCRH = 0;		// 分频值
	PWMB_PSCRL = sys_clk / 1000000 - 1;    // 分频值
    PWMB_SMCR = 0x54;	// TS=TI1FP1,SMS=TI1上升沿复位模式
	PWMB_CR1 = 0x01;	// 启动PWMB，向上计数
	PWMB_IER = 0x07;	// 使能CC1、CC2、UIE中断

    pwmin.period = 0;
    pwmin.high_value = 0;
    pwmin.high_time = 0;
}