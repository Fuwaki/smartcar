#include "STC32G.h"
#include "intrins.h"

/***** PWM端口定义 *****
 * PWM1P - P1^0 - PWMA_CH1P - 第1对PWM正端
 * PWM1N - P1^1 - PWMA_CH1N - 第1对PWM负端
 * PWM2P - P1^2 - PWMA_CH2P - 第2对PWM正端
 * PWM2N - P1^3 - PWMA_CH2N - 第2对PWM负端
 * PWM3P - P1^4 - PWMA_CH3P - 第3对PWM正端
 * PWM3N - P1^5 - PWMA_CH3N - 第3对PWM负端
 */

// 系统时钟定义
#define FOSC 12000000L  // 系统时钟12MHz
#define PWM_TIMER_PERIOD 200
float timestamp = 0;
float timestamp_previous = 0;

/**
 * @brief PWM初始化函数
 * @param freq: PWM频率，单位Hz
 * @param dead_time: 死区时间，单位为系统时钟周期数
 */
void PWM_Init(unsigned int freq, unsigned char dead_time)
{
    unsigned int arr;
    
    // 计算自动重载寄存器值
    arr = (FOSC / (freq * 2)) - 1;  // 中心对齐模式，频率=FOSC/(2*ARR+1)
    if(arr > 0xFFFF) arr = 0xFFFF;  // 防止溢出


    // 修正为推挽输出模式
    P1M0 |= 0x3F;  // P1.0-P1.5设置为1
    P1M1 &= ~0x3F; // P1.0-P1.5设置为0

    
    // 2. 关闭PWM计数器
    PWMA_CR1 = 0x00;  // 停止定时器
    
    // 3. 设置PWM时钟预分频
    PWMA_PSCRH = 0x00;  // 预分频高位
    PWMA_PSCRL = 0x00;  // 预分频低位（分频系数=1）
    
    // 4. 设置自动重载值ARR（决定PWM频率）
    PWMA_ARRH = (arr >> 8) & 0xFF;  // ARR高位
    PWMA_ARRL = arr & 0xFF;         // ARR低位
    
    // 5. 设置死区时间
    PWMA_DTR = dead_time;  // 设置死区时间
    
    // 6. 配置PWM模式
    PWMA_CCMR1 = 0x68;  // 通道1: PWM模式1(0110)，预装载使能(1000)
    PWMA_CCMR2 = 0x68;  // 通道2: PWM模式1，预装载使能
    PWMA_CCMR3 = 0x68;  // 通道3: PWM模式1，预装载使能
    
    // 7. 配置输出使能和极性
    PWMA_CCER1 = 0x55;  // 通道1和通道2使能正负输出，极性正
    PWMA_CCER2 = 0x05;  // 通道3使能正负输出，极性正
    
    // 8. 设置中心对齐模式
    PWMA_CR1 |= 0x60;  // 中心对齐模式1(边沿对齐向上计数到ARR再向下计数)
    
    // 9. 设置PWM1/PWM2/PWM3引脚为P1口
    PWMA_PS = 0x00;    // 选择PWM1/PWM2/PWM3使用P1口
    
    // 10. 启用PWM输出
    PWMA_BKR = 0x80;   // 主输出使能
    PWMA_ENO = 0x3F;   // 使能所有6个通道输出(PWM1P/1N/2P/2N/3P/3N)
    
    // 11. 启动PWM计数器
    PWMA_CR1 |= 0x01;  // 启动定时器
}

/**
 * @brief 设置PWM频率
 * @param freq: 频率，单位Hz
 * @return: 设置是否成功，0-成功，1-频率过高或过低
 */
unsigned char Set_PWM_Frequency(unsigned int freq)
{
    unsigned int arr;
    
    if(freq < 10 || freq > 100000)  // 检查频率范围
        return 1;
    
    // 计算自动重载寄存器值
    arr = (FOSC / (freq * 2)) - 1;
    if(arr > 0xFFFF) return 1;  // 频率太低
    
    // 停止计数器
    PWMA_CR1 &= ~0x01;
    
    // 设置ARR值
    PWMA_ARRH = (arr >> 8) & 0xFF;
    PWMA_ARRL = arr & 0xFF;
    
    // 重新启动计数器
    PWMA_CR1 |= 0x01;
    
    return 0;
}

/**
 * @brief 设置PWM通道占空比
 * @param channel: 通道号(1-3)
 * @param duty: 占空比(0-100)
 * @return: 设置是否成功，0-成功，1-参数错误
 */
unsigned char Set_PWM_Duty(unsigned char channel, unsigned char duty)
{
    unsigned int period, compare_value;
    
    if(channel < 1 || channel > 3 || duty > 100)  // 参数检查
        return 1;
    
    // 获取当前周期值
    period = ((unsigned int)PWMA_ARRH << 8) | PWMA_ARRL;
    
    // 计算比较值
    compare_value = (period * duty) / 100;
    
    // 设置占空比
    switch(channel)
    {
        case 1:
            PWMA_CCR1H = (compare_value >> 8) & 0xFF;
            PWMA_CCR1L = compare_value & 0xFF;
            break;
        case 2:
            PWMA_CCR2H = (compare_value >> 8) & 0xFF;
            PWMA_CCR2L = compare_value & 0xFF;
            break;
        case 3:
            PWMA_CCR3H = (compare_value >> 8) & 0xFF;
            PWMA_CCR3L = compare_value & 0xFF;
            break;
    }
    
    return 0;
}

/**
 * @brief 控制PWM通道输出使能
 * @param channel: 通道号(1-3)
 * @param enable: 1-使能, 0-禁止
 * @return: 设置是否成功，0-成功，1-参数错误
 */
unsigned char PWM_Channel_Controller(unsigned char channel, unsigned char enable)
{
    if(channel < 1 || channel > 3)  // 参数检查
        return 1;
    
    switch(channel)
    {
        case 1:
            if(enable)
            {
                PWMA_CCER1 |= 0x05;  // 使能通道1正负输出
                PWMA_ENO |= 0x03;    // 使能PWM1P和PWM1N
            }
            else
            {
                PWMA_CCER1 &= ~0x05; // 禁止通道1正负输出
                PWMA_ENO &= ~0x03;   // 禁止PWM1P和PWM1N
            }
            break;
        case 2:
            if(enable)
            {
                PWMA_CCER1 |= 0x50;  // 使能通道2正负输出
                PWMA_ENO |= 0x0C;    // 使能PWM2P和PWM2N
            }
            else
            {
                PWMA_CCER1 &= ~0x50; // 禁止通道2正负输出
                PWMA_ENO &= ~0x0C;   // 禁止PWM2P和PWM2N
            }
            break;
        case 3:
            if(enable)
            {
                PWMA_CCER2 |= 0x05;  // 使能通道3正负输出
                PWMA_ENO |= 0x30;    // 使能PWM3P和PWM3N
            }
            else
            {
                PWMA_CCER2 &= ~0x05; // 禁止通道3正负输出
                PWMA_ENO &= ~0x30;   // 禁止PWM3P和PWM3N
            }
            break;
    }
    
    return 0;
}

/**
 * @brief 设置PWM死区时间
 * @param dead_time: 死区时间，单位为系统时钟周期数
 */
void Set_PWM_DeadTime(unsigned char dead_time)
{
    PWMA_DTR = dead_time;
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