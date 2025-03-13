#include <AI8051U.H>
#include <intrins.h>

// 定义中断标志位
volatile unsigned char PWMA_IntFlag = 0;

sbit ENCODER_ZERO = P1 ^ 0;  // 编码器零点信号 Z相
unsigned char current_A = 0; // 编码器A相信号
unsigned char current_B = 0; // 编码器B相信号

// 添加变量来记录上一次的编码器状态
unsigned char last_A = 0;
unsigned char last_B = 0;

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

void Encoder_Init()
{
    PWMA_ENO = 0x00; // 禁用所有PWM输出

    PWMA_CCMR1 = 0x41; // 设置通道1为输入捕获模式(0x01)，并启用输入滤波(0x40)
    PWMA_CCMR2 = 0x41; // 设置通道2为输入捕获模式(0x01)，并启用输入滤波(0x40)

    // 设置双向捕获模式：同时捕获上升沿和下降沿
    // 0x33 = 0011 0011b，其中:
    // - 位0和4：使能通道1和通道2的捕获功能
    // - 位1和5：使能通道1和通道2的下降沿捕获
    // - 位3和7：使能通道1和通道2的上升沿捕获
    PWMA_CCER1 = 0x33; // 使能通道1和2的双向(上升沿和下降沿)捕获

    PWMA_CNTRH = 0;
    PWMA_CNTRL = 0;

    PWMA_CCR1H = 0;
    PWMA_CCR1L = 0;
    PWMA_CCR2H = 0;
    PWMA_CCR2L = 0;

    PWMA_PSCRH = 0;
    PWMA_PSCRL = 0;

    // 清除中断标志
    PWMA_SR1 = 0;
    PWMA_SR2 = 0;

    Encoder_InterruptEnable(0x01); // 使能更新中断
    Encoder_InterruptEnable(0x02); // 使能通道1捕获中断
    Encoder_InterruptEnable(0x04); // 使能通道2捕获中断

    // 启动定时器
    PWMA_CR1 = 0x01; // 使能PWM定时器

    // 清除内部中断标志
    PWMA_IntFlag = 0;

    // 初始化编码器数据
    encoder.direction = 0;
    encoder.position = 0;
    encoder.speed = 0;
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
        // 无效通道，不执行任何操作
        break;
    }
}

/**
 * 检测编码器是否处于零点位置
 * @return: 如果在零点位置返回1，否则返回0
 */
void Encoder_DetectZero(void)
{
    if (ENCODER_ZERO == 0)
    {
        // 在检测到零点信号时重置计数器
        Encoder_Clear(1);
        Encoder_Clear(2);
        encoder.position = 0;
    }
}

// 更新编码器位置和速度
void Encoder_Update(void)
{
    static int lastCount = 0;
    // 获取当前计数值
    int currentCount = Encoder_Read(1); // 读取通道1的计数值
    // 计算位置
    encoder.position += (currentCount - lastCount) * encoder.direction * 0.01f; 
    Encoder_DetectZero();
    // 更新上次计数值
    lastCount = currentCount;
}

void Encoder_InterruptDisable(unsigned char intType)
{
    PWMA_IER &= ~intType; // 禁用指定的中断
}

void PWMA_Interrupt() interrupt PWMA_VECTOR
{
    // 检查中断源
    if (PWMA_SR1 & 0x01) // 更新中断
    {
        PWMA_SR1 &= ~0x01; // 清除中断标志
        current_A = (P3 & 0x01) ? 1 : 0; // 读取P3.0状态
        current_B = (P3 & 0x02) ? 1 : 0; // 读取P3.1状态

        // 使用四状态检测法判断旋转方向
        if (last_A == 0 && last_B == 0)
        {
            if (current_A == 1 && current_B == 0)
                encoder.direction = 1; // 顺时针
            else if (current_A == 0 && current_B == 1)
                encoder.direction = -1; // 逆时针
        }
        else if (last_A == 1 && last_B == 0)
        {
            if (current_A == 1 && current_B == 1)
                encoder.direction = 1; // 顺时针
            else if (current_A == 0 && current_B == 0)
                encoder.direction = -1; // 逆时针
        }
        else if (last_A == 1 && last_B == 1)
        {
            if (current_A == 0 && current_B == 1)
                encoder.direction = 1; // 顺时针
            else if (current_A == 1 && current_B == 0)
                encoder.direction = -1; // 逆时针
        }
        else if (last_A == 0 && last_B == 1)
        {
            if (current_A == 0 && current_B == 0)
                encoder.direction = 1; // 顺时针
            else if (current_A == 1 && current_B == 1)
                encoder.direction = -1; // 逆时针
        }
        // 更新上一次的状态
        last_A = current_A;
        last_B = current_B;
        
        // 更新编码器位置和速度
        Encoder_Update();

        PWMA_IntFlag = 1;
    }

    // // 检查捕获比较通道1中断
    // if (PWMA_SR1 & 0x02)
    // {
    //     PWMA_SR1 &= ~0x02;  // 清除中断标志
    // }

    // // 检查捕获比较通道2中断
    // if (PWMA_SR1 & 0x04)
    // {
    //     PWMA_SR1 &= ~0x04;  // 清除中断标志
    // }
}