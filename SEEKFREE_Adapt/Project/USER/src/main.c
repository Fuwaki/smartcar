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
#define PWM_A_H_PIN P13
#define PWM_A_L_PIN P54

#define PWM_B_H_PIN P17
#define PWM_B_L_PIN P16

#define PWM_C_H_PIN P15
#define PWM_C_L_PIN P14
// 引脚定义（根据实际硬件修改）
#define UH PWM_A_H_PIN // U相上桥臂
#define UL PWM_A_L_PIN
#define VH PWM_B_H_PIN // V相上桥臂
#define VL PWM_B_L_PIN
#define WH PWM_C_H_PIN // W相上桥臂
#define WL PWM_C_L_PIN

void delay(unsigned int t)
{
    while (t--); // 简易延时
}

// 关闭所有桥臂（重要安全措施）
void all_off()
{
    UH = 0;
    UL = 1;
    VH = 0;
    VL = 1;
    WH = 0;
    WL = 1;
}

void a() {
    unsigned char step = 0;

    while(1) {
        all_off(); // 先关闭所有输出
        delay(300); // 调速延时

        switch(step) {
            case 0: // Step1: U+ V-
                UH = 1;  // 上桥臂导通
                VL = 0;  // 下桥臂导通
                break;
            case 1: // Step2: U+ W-
                UH = 1;
                WL = 0;
                break;
            case 2: // Step3: V+ W-
                VH = 1;
                WL = 0;
                break;
            case 3: // Step4: V+ U-
                VH = 1;
                UL = 0; 
                break;
            case 4: // Step5: W+ U-
                WH = 1;
                UL = 0;
                break;
            case 5: // Step6: W+ V-
                WH = 1;
                VL = 0;
                break;
        }

        if(++step > 5) step = 0;  // 循环6步 

        delay(3000); // 调速延时
    }
}


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

    P42 = 0;
    pwm_close_output();
    a();

    // download_flag = 0;

    // 此处编写用户代码(例如：外设初始化代码等)
    battery_init();         // 电池电压检测初始化
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
