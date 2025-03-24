#include <AI8051U.H>
#include <math.h>
#include "Timer.h"
#include <intrins.h>

sbit ENCODER_PULSE = P1 ^ 2; // 编码器脉冲信号 - PWMA通道2捕获引脚
sbit ENCODER_DIR = P1 ^ 0;   // 编码器方向信号 1为正转，0为反转
sbit ENCODER_ZERO = P0 ^ 1;  // 编码器零点信号 Z相

// 编码器常量定义
#define PWM_PSC 1                                // PWM预分频系数
#define ENCODER_LINES 1024                       // 编码器线数
#define ANGLE_PER_PULSE (180.0f / ENCODER_LINES) // 每个脉冲的角度，除以2是因为上升沿和下降沿都会计数

// unsigned char ENABLE_ZERO_DETECT = 1; // 是否启用零点检测
float timestamp_previous = 0; // 上一个时间戳
static int lastPosition = 0;
static int lastCount = 0;
static unsigned long totalPulses = 0; // 用于记录总脉冲数

struct EncoderData
{
    unsigned char direction; // 编码器旋转方向
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
    PWMA_PSCRH = (int)(PWM_PSC >> 8); // 设置PWM预分频系数
    PWMA_PSCRL = (int)(PWM_PSC);

    PWMA_ENO = 0x00; // 禁用所有PWM输出

    //PWMA_CCMR1 = 0x01; // 不再设置通道1为输入捕获模式
    PWMA_CCMR2 = 0x01; // 设置通道2为输入捕获模式(0x01)，如果想启用输入滤波就+(0x40)

    // 只设置通道2捕获模式：只捕获上升沿和下降沿
    // 0x30 = 0011 0000b，其中:
    // - 位4：使能通道2的捕获功能
    // - 位5：使能通道2的下降沿捕获
    // - 位6：使能通道2的上升沿捕获
    PWMA_CCER1 = 0x70; // 0x70 (0111 0000b)，同时启用位4、位5和位6

    PWMA_CNTRH = 0;
    PWMA_CNTRL = 0;

    // 不再初始化通道1捕获寄存器
    //PWMA_CCR1H = 0;
    //PWMA_CCR1L = 0;
    
    PWMA_CCR2H = 0; // 初始化通道2捕获寄存器高位
    PWMA_CCR2L = 0; // 初始化通道2捕获寄存器低位

    // 清除中断标志
    PWMA_SR1 = 0;
    PWMA_SR2 = 0;

    Encoder_InterruptEnable(0x01); // 使能更新中断
    //Encoder_InterruptEnable(0x02); // 不再使能通道1捕获中断
    Encoder_InterruptEnable(0x04); // 使能通道2捕获中断

    // 启动定时器
    PWMA_CR1 = 0x01; // 使能PWM定时器

    // 初始化编码器数据
    encoder.direction = 0;
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
    if (ENCODER_ZERO == 0)
    {
        Encoder_Clear(2); // 清零通道2 (之前是通道1)
        lastPosition = 0;
        encoder.position = 0;
    }
}

// 更新编码器位置和速度
void Encoder_Update()
{
    // 获取当前计数值
    int currentCount = Encoder_Read(2); // 读取通道2的计数值 (之前是通道1)
    int deltaPulses = currentCount - lastCount;

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
    totalPulses += fabs(deltaPulses);

    // 计算位置(1或-1)表示正反转
    encoder.position += deltaPulses * encoder.direction * ANGLE_PER_PULSE;

    _normalizeAngle(encoder.position); // 规范化角度值

    // 检测零点
    Encoder_DetectZero();

    // 假设更新周期是固定的，可以在这里计算速度
    // 如果中断频率是1ms
    // encoder.speed = deltaPulses * encoder.direction * ANGLE_PER_PULSE * 1000; // 角度/秒
    encoder.speed = (encoder.position - lastPosition)/(timestamp - timestamp_previous); //dtheta/dt

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
    }
}

//Ciallo～(∠・ω< )⌒★!!!
//Ciallo～(∠・ω< )⌒★!!!
//Ciallo～(∠・ω< )⌒★!!!

//Fuwaki u damn boy!!!