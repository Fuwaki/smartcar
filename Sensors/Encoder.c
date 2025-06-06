#include <AI8051U.H>
#include <math.h>
#include "Timer.h"
#include <intrins.h>

sbit ENCODER_PULSE = P1 ^ 2; // 编码器脉冲信号 - PWMA通道2捕获引脚
sbit ENCODER_DIR = P1 ^ 0;   // 编码器方向信号 1为正转，0为反转
sbit ENCODER_ZERO = P0 ^ 1;  // 编码器零点信号 Z相
sbit ENCODER_ENABLE = P0^3; // 编码器使能信号 - 1为使能，0为禁用

// 编码器常量定义
#define PWM_PSC 0                                // PWM预分频系数
#define ENCODER_LINES 1024                       // 编码器线数
#define ANGLE_PER_PULSE (3.1415f / ENCODER_LINES) // 每个脉冲的角度，除以2是因为上升沿和下降沿都会计数

// unsigned char ENABLE_ZERO_DETECT = 1; // 是否启用零点检测
float timestamp_previous = 0; // 上一个时间戳
static int lastPosition = 0;
static int lastCount = 0;
static unsigned int totalCount = 0; // 用于记录上次计数值
// static unsigned long totalPulses = 0; // 用于记录总脉冲数

struct EncoderData
{
    char direction; // 编码器旋转方向
    float position;          // 编码器位置
    float speed;             // 编码器速度
};

struct EncoderData encoder;

/**
 * 使能PWM定时器中断
 * @param intType: 中断类型，可以是以下值的组合:
 *                 0x01: 更新中断
 *                 0x02: 捕获/比较通道1中断
 *                 0x04: 捕获/比较通道2中断
 *                 0x08: 捕获/比较通道3中断
 *                 0x10: 捕获/比较通道4中断
 *                 0x20: COM中断
 *                 0x40: 触发中断
 *                 0x80: 断路中断
 */
void Encoder_InterruptEnable(unsigned char intType)
{
    P_SW2 |= 0x80; // 设置EAXFR为1，使能扩展RAM区域

    PWMA_IER |= intType; // 使能指定的中断
    EA = 1;              // 使能总中断
    PPWMAH = 1;          // 设置高优先级
    PPWMA = 1;
}

void Encoder_InterruptDisable(unsigned char intType)
{
    PWMA_IER &= ~intType; // 禁用指定的中断
}

float _normalizeAngle(float angle)
{
    float a = fmod(angle, 360.0f);
    return a >= 0 ? a : (a + 360.0f);
}

void Encoder_Init()
{
    P_SW2 |= 0x80; // 设置EAXFR为1，使能扩展RAM区域
    P1M0 &= ~0x07; P1M1 |= 0x07;  // 设置P1.0、P1.1、P1.2为开漏输入
    P0M0 |= 0x08; P0M1 &= ~0x08; // 设置P0.3为推挽输出
    ENCODER_ENABLE = 1; // 使能编码器
    PWMA_PSCRH = (int)(PWM_PSC >> 8); // 设置PWM预分频系数
    PWMA_PSCRL = (int)(PWM_PSC);

    PWMA_ENO = 0x00; // 禁用所有PWM输出

    PWMA_CCER1 = 0x00; // 禁用通道1捕获功能

    PWMA_CCMR2 = 0x41; // 设置通道2为输入捕获模式(0x01)，如果想启用输入滤波就+(0x40)

    // 只设置通道2捕获模式：只捕获上升沿和下降沿
    // 0x30 = 0011 0000b，其中:
    // - 位4：使能通道2的捕获功能
    // - 位5：使能通道2的下降沿捕获
    // - 位6：使能通道2的上升沿捕获
    PWMA_CCER1 = 0x70; // 0x70 (0111 0000b)，同时启用位4、位5和位6

    PWMA_CNTRH = 0;
    PWMA_CNTRL = 0;

    PWMA_ARRH = 0xff; // 设置自动重载寄存器高位
    PWMA_ARRL = 0xff; // 设置自动重载寄存器低位

    PWMA_CCR1H = 0;
    PWMA_CCR1L = 0;
    
    PWMA_CCR2H = 0; // 初始化通道2捕获寄存器高位
    PWMA_CCR2L = 0; // 初始化通道2捕获寄存器低位

    EA = 1; // 使能总中断

    // 清除中断标志
    PWMA_SR1 = 0;
    PWMA_SR2 = 0;

    Encoder_InterruptEnable(0x01); // 使能更新中断
    //Encoder_InterruptEnable(0x02); // 不使能通道1捕获中断
    Encoder_InterruptEnable(0x04); // 使能通道2捕获中断 - 重要！需要启用此中断

    // 启动定时器
    PWMA_CR1 = 0x01; // 使能PWM定时器

    // 初始化编码器数据
    encoder.direction = ENCODER_DIR ? 1 : -1;
    encoder.position = 0;
    encoder.speed = 0;

    lastPosition = encoder.position;
    timestamp_previous = timestamp;
}

/*读取编码器计数值*/
int Encoder_Read(unsigned char channel)
{
    int encoderValue = 0;
    switch (channel)
    {
    case 1:
        // 读取通道1的捕获值
        encoderValue = (PWMA_CCR1H << 8) | PWMA_CCR1L;
        break;
    case 2:
        // 读取通道2的捕获值
        encoderValue = (PWMA_CCR2H << 8) | PWMA_CCR2L;
        break;
    default:
        // 无效通道，返回0
        return 0;
    }

    return encoderValue;
}

/*清零编码器计数值*/
void Encoder_Clear(unsigned char channel)
{
    switch (channel)
    {
    case 1:
        PWMA_CCR1H = 0;
        PWMA_CCR1L = 0;
        break;
    case 2:
        PWMA_CCR2H = 0;
        PWMA_CCR2L = 0;
        break;
    default:
        break;
    }
    PWMA_CNTRH = 0;
    PWMA_CNTRL = 0;
}

void Encoder_DetectZero(void)
{
    if (ENCODER_ZERO == 1)
    {
        Encoder_Clear(2); // 清零通道2
        lastPosition = 0;
        encoder.position = 0;
        lastCount = 0;
        // totalPulses = 0; // 清零总脉冲数
        lastPosition = 0; // 清零上次位置
        //UART_SendStr("Encoder Zero Detected!\n"); // 调试信息
    }
}

// 更新编码器位置和速度
void Encoder_Update()
{
    // 获取当前计数值
    int currentCount;
    int deltaPulses;

    currentCount =  Encoder_Read(2); // 读取通道2的计数值
    deltaPulses = currentCount - lastCount; // 计算增量脉冲数

    // 处理溢出情况
    if (deltaPulses > 32767)
    {
        deltaPulses -= 65536;
    }
    else if (deltaPulses < -32767)
    {
        deltaPulses += 65536;
    }

    // 更新总脉冲数
    // totalPulses += (deltaPulses > 0) ? deltaPulses : -deltaPulses; lmaoing
    // totalPulses += fabs(deltaPulses);

    // 计算位置(1或-1)表示正反转
    encoder.position += deltaPulses * encoder.direction * ANGLE_PER_PULSE;

    // _normalizeAngle(encoder.position); // 规范化角度值
    //TODO: 到底是否需要规范化角度值？
    //? 这个地方的规范化角度值是为了防止角度，但是在零点检测的时候会清零，所以这里不需要规范化
    // 检测零点
    Encoder_DetectZero();

    // 假设更新周期是固定的，可以在这里计算速度
    // 如果中断频率是1ms
    // encoder.speed = deltaPulses * encoder.direction * ANGLE_PER_PULSE * 1000; // 角度/秒
    encoder.speed = (encoder.position - lastPosition)/(timestamp - timestamp_previous); //d theta/dt
    // encoder.speed = encoder.position; //d theta/dt


    // 更新上次计数值
    lastCount = currentCount;
    timestamp_previous = timestamp;
    lastPosition = encoder.position;
}

void PWMA_Interrupt() interrupt PWMA_VECTOR
{
    // 检查中断源
    if (PWMA_SR1 & 0x01) // 更新中断
    {
        PWMA_SR1 &= ~0x01; // 清除中断标志

        encoder.direction = ENCODER_DIR ? 1 : -1; // bro别看错了

        Encoder_Update();
    }

    // 处理通道2捕获中断 - 用于计数脉冲
    if (PWMA_SR1 & 0x04)  // 捕获比较通道2中断
    {
        PWMA_SR1 &= ~0x04;  // 清除中断标志
        if (encoder.direction == 1)
        {
            totalCount ++;
        }
        else
        {
            totalCount --;
        }

        if (totalCount >= 2048)
        {
            Encoder_DetectZero(); // 检测零点
            totalCount = 0; // 清零计数器
        }

        // 当发生捕获事件时，我们可以在此处理脉冲计数
        // 但实际上计数已经由PWMA硬件自动完成，我们只需确保中断标志被清除
    }
}

//Ciallo～(∠・ω< )⌒★!!!
//Ciallo～(∠・ω< )⌒★!!!
//Ciallo～(∠・ω< )⌒★!!!
//☆⌒( >ω・∠)～ollɐıɔ
//Ciallo ～(∠・ω< )⌒★!!!

//Fuwaki u damn boy!!!