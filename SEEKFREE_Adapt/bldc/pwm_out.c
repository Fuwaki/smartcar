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

#include "intrins.h"
#include "zf_gpio.h"
#include "bldc_config.h"
#include "comparator.h"
#include "pwm_out.h"

uint8 g_use_complementary = 0;

//-------------------------------------------------------------------------------------------------------------------
//  @brief      软延时
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
//  @brief      刹车（关闭输出并开启所有下桥）
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
//  @brief      关闭输出
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
//  @brief      开启A上B下
//  @param      void
//  @return     void
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
// A对应PWM2
void pwm_a_bn_output(uint8 use_comp)

{
	pwm_close_output();
	if (use_comp)
	{
		// 开启PWM输出
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
//  @brief      开启A上C下
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
//  @brief      开启B上C下
//  @param      void
//  @return     void
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
// B通道在PWM3
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
//  @brief      开启B上A下
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
//  @brief      开启C上A下
//  @param      void
//  @return     void
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
// C通道在PWM4
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
//  @brief      开启C上B下
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
////  @brief      关闭PWM中断
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
////  @brief      开启PWM中断
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
//  @brief      更新PWM占空比
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
	// 字节拆分 轮流写入

	PWMA_CCR1H = temp_h;
	PWMA_CCR1L = temp_l;

	PWMA_CCR2H = temp_h;
	PWMA_CCR2L = temp_l;

	PWMA_CCR3H = temp_h;
	PWMA_CCR3L = temp_l;
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      PWM初始化（中心对齐方式）
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

	// 初始化低边为推挽输出
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

	// 设置PWM引脚
	// Pin Switch
	PWMA_PS = 0x00; // 00000000  配置pwm
	// Capture Compare Mode Register
	// 通道B A相
	PWMA_CCMR2 = 0x78; // 通道模式配置, PWM模式2, 预装载允许
	// CCMR pwm模式
	// Capture Compare Register
	PWMA_CCR2H = 0;
	PWMA_CCR2L = 0;
	// Capture Compare Enable Register
	PWMA_CCER1 |= 0x30; // 开启比较输出, 低电平有效
	// 通道C B相
	// FIXME 逐飞你是不是搞错了 明明都是PWM模式1啊
	PWMA_CCMR3 = 0x78; // 通道模式配置, PWM模式1, 预装载允许
	PWMA_CCR3H = 0;
	PWMA_CCR3L = 0;
	PWMA_CCER2 |= 0x03; // 开启比较输出, 低电平有效
	// 通道D C相
	PWMA_CCMR4 = 0x78; // 通道模式配置, PWM模式1, 预装载允许
	PWMA_CCR4H = 0;
	PWMA_CCR4L = 0;
	PWMA_CCER2 |= 0x30; // 开启比较输出, 低电平有效

	//    PWMA_IER    = 0x10;     // 开启更新中
	// PWMA_ENO = 0xff;//0X15;

	// 预分频
	PWMA_PSCRH = 0;
	PWMA_PSCRL = 0;

	// 设置周期
	PWMA_ARRH = (uint8)((BLDC_MAX_DUTY - 1) >> 8);
	PWMA_ARRL = (uint8)((BLDC_MAX_DUTY - 1) & 0xff);

	PWMA_BKR = 0x80; // 主输出使能 相当于总开关
	PWMA_CR1 = 0x85; // 使能计数器, 允许自动重装载寄存器缓冲, 边沿对齐, 向上计数, 只有计数器上下溢出才触发更新中断,  bit7=1:写自动重装载寄存器缓冲(本周期不会被打扰), =0:直接写自动重装载寄存器本(周期可能会乱掉)
	PWMA_EGR = 0x01; // 产生一次更新事件, 清除计数器和与分频计数器, 装载预分频寄存器的值

	// 刚开始使用刹车将
	PWM_A_L_PIN = 1;
	PWM_B_L_PIN = 1;
	PWM_C_L_PIN = 1;
}
