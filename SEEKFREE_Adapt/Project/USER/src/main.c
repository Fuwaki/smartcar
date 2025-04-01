/*********************************************************************************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2020,逐飞科技
 * All rights reserved.
 * 技术讨论QQ群：一群：179029047(已满)  二群：244861897(已满)  三群：824575535
 *
 * 以下所有内容版权均属逐飞科技所有，未经允许不得用于商业用途，
 * 欢迎各位使用并传播本程序，修改内容时必须保留逐飞科技的版权声明。
 *
 * @file       		main
 * @company	   		成都逐飞科技有限公司
 * @author     		逐飞科技(QQ790875685)
 * @version    		查看doc内version文件 版本说明
 * @Software 		MDK FOR C251 V5.60
 * @Target core		STC32G12K128
 * @Taobao   		https://seekfree.taobao.com/
 * @date       		2020-12-18
 ********************************************************************************************************************/
#include "motor_control.h"
#include "comparator.h"
#include "bldc_config.h"
#include "pwm_out.h"
#include "signal_input.h"
#include "pit_timer.h"
#include "battery.h"
#include "headfile.h"

// 关于内核频率的设定，可以查看board.h文件
// 在board_init中,已经将P54引脚设置为复位
// 如果需要使用P54引脚,可以在board.c文件中的board_init()函数中删除SET_P54_RESRT即可

// 无刷电机LED状态灯说明
// 电池电压过低时，             LED亮0.1s    灭0.9s
// 电机遇到堵转，               LED亮0.05s   灭0.05s
// 电机未运行时，               LED亮1s      灭1s
// 电机开环启动中，             LED亮0.5s    灭0.5s
// 电机开环启动完成等待稳定中， LED亮0.1s    灭0.1s
// 电机正常运行中，             LED常亮亮

// 旧电调需要拆掉C27、C28、C29三颗滤波电容
// 旧电调需要拆掉C27、C28、C29三颗滤波电容
// 旧电调需要拆掉C27、C28、C29三颗滤波电容



void main()
{
    CKCON = 0;
    WTST = 0;           // 设置程序代码等待参数，赋值为0可将CPU执行程序的速度设置为最快
    DisableGlobalIRQ(); // 关闭总中断
    sys_clk = 35000000; // 设置系统频率为35MHz
    board_init();       // 初始化寄存器
    gpio_mode(P5_4, GPO_PP);
    gpio_mode(P1_3, GPO_PP);
    gpio_mode(P1_7, GPO_PP);
    gpio_mode(P1_6, GPO_PP);
    gpio_mode(P1_5, GPO_PP);
    gpio_mode(P1_4, GPO_PP);


    // download_flag = 0;

    // 此处编写用户代码(例如：外设初始化代码等)
    // battery_init();         // 电池电压检测初始化
    led_init();             // LED初始化
    pwm_input_init();       // PWM输入捕获初始化·
    comparator_init();      // 比较器初始化
    motor_init();           // 电机相关初始化

    // PWM初始化务必放在电机电机初始化函数之后，否则会烧毁电机
    // PWM初始化务必放在电机电机初始化函数之后，否则会烧毁电机
    // PWM初始化务必放在电机电机初始化函数之后，否则会烧毁电机
    
    pwm_out_init();      	// PWM初始化 采用中心对齐
    pit_timer_init();       // 周期定时器初始化
    EnableGlobalIRQ();		// 开启总中断

    while (1)
    {
    }
}
