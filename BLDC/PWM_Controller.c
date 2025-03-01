#include <AI8051U.H>
#include <intrins.h>

#define FOSC 12000000L  // 系统时钟12MHz
#define PWM_TIMER_PERIOD 20000

#pragma region PWM_Controller
// PWM输出引脚定义
sbit PWM1P = P1^0;  // PWM通道1正端
sbit PWM1N = P1^1;  // PWM通道1负端
sbit PWM2P = P1^2;  // PWM通道2正端
sbit PWM2N = P1^3;  // PWM通道2负端
sbit PWM3P = P1^4;  // PWM通道3正端
sbit PWM3N = P1^5;  // PWM通道3负端

// PWM全局变量
unsigned char PWM_DUTY0;  // 通道1占空比值(0-100)
unsigned char PWM_DUTY1;  // 通道2占空比值(0-100)
unsigned char PWM_DUTY2;  // 通道3占空比值(0-100)
unsigned int PWM_PERIOD;  // PWM周期值(ARR寄存器值)
#pragma endregion

//时间戳,为提供systick
double timestamp;
double timestamp_previous;

/**
 * PWM初始化函数 - 配置PWM为中心对齐互补模式
 * @param freq: PWM频率，单位Hz
 * @param dead_time: 死区时间，单位为系统时钟周期数
 */
void PWM_Init(unsigned int freq, unsigned char dead_time)
{
    unsigned long arr;
    
    // 计算自动重载寄存器值
    arr = (FOSC / (freq * 2)) - 1;  // 中心对齐模式，频率=FOSC/(2*ARR+1)
    if(arr > 0xFFFF) arr = 0xFFFF;  // 防止溢出
    
    // 保存周期值
    PWM_PERIOD = arr;
    
    // 初始化占空比为0
    PWM_DUTY0 = 0;
    PWM_DUTY1 = 0;
    PWM_DUTY2 = 0;
    
    // 1. 停止计数器
    PWMA_CR1 = 0x00;  // 停止计数器
    
    // 2. 配置PWM时钟预分频（默认为系统时钟）
    PWMA_PSCRH = 0x00;  // 预分频高位
    PWMA_PSCRL = 0x00;  // 预分频低位（分频系数=1）
    
    // 3. 配置自动重载寄存器（ARR）初值
    PWMA_ARRH = (arr >> 8) & 0xFF;  // ARR高位
    PWMA_ARRL = arr & 0xFF;         // ARR低位
    
    // 4. 配置死区时间
    PWMA_DTR = dead_time;  // 死区时间设置
    
    // 5. 配置通道1-3为PWM模式1，预装载使能
    PWMA_CCMR1 = 0x68;  // 通道1: PWM模式1(0110)，预装载使能(1000)
    PWMA_CCMR2 = 0x68;  // 通道2: PWM模式1，预装载使能
    PWMA_CCMR3 = 0x68;  // 通道3: PWM模式1，预装载使能
    
    // 6. 使能互补输出
    // 0x55 = 0101 0101, 表示通道1-3的正负端均使能
    PWMA_CCER1 = 0x55;  // 通道1-2使能正负输出
    PWMA_CCER2 = 0x55;  // 通道3使能正负输出
    
    // 7. 配置中心对齐模式和计数方向
    PWMA_CR1 |= 0x20;  // 中心对齐模式1
    
    // 8. 使能PWM输出
    PWMA_BKR = 0x80;  // 主输出使能
    PWMA_ENO = 0x3F;  // 使能所有6个输出通道(0011 1111)
    
    // 9. 启动计数器
    PWMA_CR1 |= 0x01;  // 启动计数器
}

/**
 * 设置PWM频率
 * @param freq: 频率，单位Hz
 */
void Set_PWM_Frequency(unsigned int freq)
{
    unsigned long arr;
    
    // 停止计数器
    PWMA_CR1 &= ~0x01;
    
    // 计算自动重载寄存器值
    arr = (FOSC / (freq * 2)) - 1;  // 中心对齐模式下的计算
    if(arr > 0xFFFF) arr = 0xFFFF;  // 防止溢出
    
    // 保存周期值
    PWM_PERIOD = arr;
    
    // 设置ARR寄存器
    PWMA_ARRH = (arr >> 8) & 0xFF;
    PWMA_ARRL = arr & 0xFF;
    
    // 重新启动计数器
    PWMA_CR1 |= 0x01;
}

/**
 * 设置PWM通道占空比
 * @param channel: 通道号(0-2)
 * @param duty: 占空比(0-100)
 */
void Set_PWM_Duty(unsigned char channel, unsigned char duty)
{
    unsigned int compare_value;
    
    // 限制占空比范围
    if(duty > 100) duty = 100;
    
    // 计算比较值
    compare_value = (PWM_PERIOD * duty) / 100;
    
    // 根据通道选择对应寄存器
    switch(channel) {
        case 0:
            PWM_DUTY0 = duty;
            PWMA_CCR1H = (compare_value >> 8) & 0xFF;
            PWMA_CCR1L = compare_value & 0xFF;
            break;
        case 1:
            PWM_DUTY1 = duty;
            PWMA_CCR2H = (compare_value >> 8) & 0xFF;
            PWMA_CCR2L = compare_value & 0xFF;
            break;
        case 2:
            PWM_DUTY2 = duty;
            PWMA_CCR3H = (compare_value >> 8) & 0xFF;
            PWMA_CCR3L = compare_value & 0xFF;
            break;
    }
}

/**
 * 启用/禁用PWM通道
 * @param channel: 通道号(0-2)
 * @param enable: 1=启用, 0=禁用
 */
void PWM_Channel_Controller(unsigned char channel, unsigned char enable)
{
    unsigned char mask = 0;
    
    // 设置通道掩码
    switch(channel) {
        case 0:
            mask = 0x03;  // 通道1的正负端
            break;
        case 1:
            mask = 0x0C;  // 通道2的正负端
            break;
        case 2:
            mask = 0x30;  // 通道3的正负端
            break;
    }
    
    // 根据enable参数启用或禁用通道
    if(enable) {
        if(channel < 2)
            PWMA_CCER1 |= mask;
        else
            PWMA_CCER2 |= mask;
    } else {
        if(channel < 2)
            PWMA_CCER1 &= ~mask;
        else
            PWMA_CCER2 &= ~mask;
    }
}

//定时器部分
void Timer0_Init()
{
    TMOD |= 0x01;
    TH0 = 0xFF;
    TL0 = 0x00; 
    ET0 = 1;
    EA = 1;
    TR0 = 1;
}

void Timer1_Init(void)
{
    TMOD |= 0x10;
    TH1 = 0xFF;
    TL1 = 0x00; 
    ET1 = 1;
    TR1 = 1;
}

void Timer0_ISR(void) interrupt 1
{
    static unsigned char pwmCounter1 = 0;
    TH0 = 0xFF;
    TL0 = 0x00;

    timestamp_previous = timestamp;
    timestamp = timestamp + 0.01;

    pwmCounter1++;
    if (pwmCounter1 >= PWM_TIMER_PERIOD)
    {
        pwmCounter1 = 0;
    }

}

void Timer1_ISR(void) interrupt 3
{
    //place ur code when you want to use second timer!
}