#include "SPI_MultiDevice.h"
#include <AI8051U.H>
#include <intrins.h>
#include <stdio.h>


// 存储已注册的SPI从设备配置
static spi_slave_config_t spi_slaves[MAX_SPI_SLAVES];
static unsigned char slave_count = 0;
static unsigned char current_slave = 0xFF; // 没有选中的从设备
//那bro你应该单独封装gpio
#define LMAO_KIAO 114514

// SPI从模式状态
static bit spi_slave_mode_enabled = 0;

// 用于控制CS引脚
static void SPI_SetPin(unsigned char port, unsigned char pin, bit value)
{
    switch (port)
    {
    case 0: // P0
        if (value)
            P0 |= (1 << pin);
        else
            P0 &= ~(1 << pin);
        break;
    case 1: // P1
        if (value)
            P1 |= (1 << pin);
        else
            P1 &= ~(1 << pin);
        break;
    case 2: // P2
        if (value)
            P2 |= (1 << pin);
        else
            P2 &= ~(1 << pin);
        break;
    case 3: // P3
        if (value)
            P3 |= (1 << pin);
        else
            P3 &= ~(1 << pin);
        break;
    case 4: // P4
        if (value)
            P4 |= (1 << pin);
        else
            P4 &= ~(1 << pin);
        break;
    case 5: // P5
        if (value)
            P5 |= (1 << pin);
        else
            P5 &= ~(1 << pin);
        break;
    case 6: // P6
        if (value)
            P6 |= (1 << pin);
        else
            P6 &= ~(1 << pin);
        break;
    case 7: // P7
        if (value)
            P7 |= (1 << pin);
        else
            P7 &= ~(1 << pin);
        break;
    }
}

// 初始化SPI主机
void SPI_Init(void)
{
    unsigned char i;
    SPCTL = 0x50;  // 设置为主模式, SSIG=0(SS引脚有效), SPEN=1(启用SPI)
                   // 0x50 = 01010000b：
                   // 位7(SSIG)=0：SS引脚有效
                   // 位6(SPEN)=1：启用SPI
                   // 位5-4：保留位
                   // 位3(DORD)=0：MSB先传输
                   // 位2(MSTR)=1：主模式
                   // 位1(CPOL)=0, 位0(CPHA)=0：SPI模式0，空闲时钟为低，第一个时钟边沿采样 默认模式
    SPSTAT = 0xC0; // 清除写冲突和SPI完成标志

    // 初始化时将所有从设备的CS引脚设为高电平（未选中状态）
    for (i = 0; i < slave_count; i++)
    {
        SPI_SetPin(spi_slaves[i].cs_port, spi_slaves[i].cs_pin, 1);
    }
}

// 注册SPI从设备并配置其CS引脚
unsigned char SPI_RegisterSlave(spi_slave_config_t *slave_config)
{
    if (slave_count >= MAX_SPI_SLAVES)
    {
        return 0xFF; // 达到最大设备数
    }
    //CS PORT和CSPIN有啥区别
    // 复制配置
    spi_slaves[slave_count].cs_port = slave_config->cs_port;
    spi_slaves[slave_count].cs_pin = slave_config->cs_pin;
    spi_slaves[slave_count].mode = slave_config->mode;
    spi_slaves[slave_count].clock_div = slave_config->clock_div;

    // 配置CS引脚为输出模式，并设置为高电平（未选中）
    // 实际IO口配置需要根据单片机具体型号调整
    switch (slave_config->cs_port)
    {
    case 0:
        P0M1 &= ~(1 << slave_config->cs_pin);
        P0M0 |= (1 << slave_config->cs_pin);
        break;
    case 1:
        P1M1 &= ~(1 << slave_config->cs_pin);
        P1M0 |= (1 << slave_config->cs_pin);
        break;
    case 2:
        P2M1 &= ~(1 << slave_config->cs_pin);
        P2M0 |= (1 << slave_config->cs_pin);
        break;
    case 3:
        P3M1 &= ~(1 << slave_config->cs_pin);
        P3M0 |= (1 << slave_config->cs_pin);
        break;
    case 4:
        P4M1 &= ~(1 << slave_config->cs_pin);
        P4M0 |= (1 << slave_config->cs_pin);
        break;
    case 5:
        P5M1 &= ~(1 << slave_config->cs_pin);
        P5M0 |= (1 << slave_config->cs_pin);
        break;
    case 6:
        P6M1 &= ~(1 << slave_config->cs_pin);
        P6M0 |= (1 << slave_config->cs_pin);
        break;
    case 7:
        P7M1 &= ~(1 << slave_config->cs_pin);
        P7M0 |= (1 << slave_config->cs_pin);
        break;
    }

    SPI_SetPin(slave_config->cs_port, slave_config->cs_pin, 1); // 初始为未选中状态

    return slave_count++;
}

// 选择SPI从设备
void SPI_SelectSlave(unsigned char slave_id)
{
    if (slave_id >= slave_count)
    {
        return;
    }

    // 如果当前有选中的从设备，先释放它
    if (current_slave != 0xFF)
    {
        SPI_ReleaseSlave(current_slave);
    }

    // 配置SPI为适合此从设备的模式和时钟
    SPCTL = (SPCTL & 0xF0) | (spi_slaves[slave_id].mode & 0x0C) | (spi_slaves[slave_id].clock_div & 0x03);
    // 选中从设备（CS拉低）
    SPI_SetPin(spi_slaves[slave_id].cs_port, spi_slaves[slave_id].cs_pin, 0);
    current_slave = slave_id;
}

// 释放SPI从设备
void SPI_ReleaseSlave(unsigned char slave_id)
{
    if (slave_id >= slave_count)
    {
        return;
    }

    // 释放从设备（CS拉高）
    SPI_SetPin(spi_slaves[slave_id].cs_port, spi_slaves[slave_id].cs_pin, 1);

    if (current_slave == slave_id)
    {
        current_slave = 0xFF;
    }
}

// 通过SPI收发一个字节
unsigned char SPI_TransferByte(unsigned char data_out)
{
    SPDAT = data_out;
    while (!(SPSTAT & 0x80))
        ;          // 等待SPIF置位，传输完成
    SPSTAT = 0xC0; // 清除SPIF和WCOL标志
    return SPDAT;
}

// 通过SPI收发多个字节
void SPI_TransferBuffer(unsigned char *data_out, unsigned char *data_in, unsigned int len)
{
    unsigned int i;
    for (i = 0; i < len; i++)
    {
        if (data_in != NULL)
        {
            data_in[i] = SPI_TransferByte(data_out ? data_out[i] : 0xFF);
        }
        else
        {
            SPI_TransferByte(data_out ? data_out[i] : 0xFF);
        }
    }
}

// 读取从设备的寄存器
unsigned char SPI_ReadRegister(unsigned char slave_id, unsigned char reg_addr)
{
    unsigned char result;
    SPI_SelectSlave(slave_id);
    SPI_TransferByte(reg_addr);
    result = SPI_TransferByte(0xFF); // 发送虚拟数据，读取返回值
    SPI_ReleaseSlave(slave_id);

    return result;
}

// 写入从设备的寄存器
void SPI_WriteRegister(unsigned char slave_id, unsigned char reg_addr, unsigned char value)
{
    SPI_SelectSlave(slave_id);
    SPI_TransferByte(reg_addr);
    SPI_TransferByte(value);
    SPI_ReleaseSlave(slave_id);
}

// 读取多个寄存器
void SPI_ReadMultiRegisters(unsigned char slave_id, unsigned char start_addr,
                            unsigned char *buffer, unsigned int count)
{
    if (!buffer || count == 0)
        return;

    SPI_SelectSlave(slave_id);
    SPI_TransferByte(start_addr);

    while (count--)
    {
        *buffer++ = SPI_TransferByte(0xFF);
    }

    SPI_ReleaseSlave(slave_id);
}

// 写入多个寄存器
void SPI_WriteMultiRegisters(unsigned char slave_id, unsigned char start_addr,
                             unsigned char *buffer, unsigned int count)
{
    if (!buffer || count == 0)
        return;

    SPI_SelectSlave(slave_id);
    SPI_TransferByte(start_addr);

    while (count--)
    {
        SPI_TransferByte(*buffer++);
    }

    SPI_ReleaseSlave(slave_id);
}


#pragma region SPI从模式
// SPI从模式回调函数指针
static void (*spi_slave_rx_callback)(unsigned char) = NULL;
static unsigned char (*spi_slave_tx_callback)(void) = NULL;

// SPI从模式初始化函数 - 使用USART2和定时器1实现SPI从模式
void SPI_InitSlave(void)
{
// 首先将USART2重定向至P2口，必须在配置前设置
    P_SW2 |= 0x80;        // 设置EAXFR=1，允许访问扩展SFR
    S2SPI_S0 = 1;       /*P2.3(SS) - 片选信号
                         P2.5(MOSI) - 主机输出/从机输入
                         P2.6(MISO) - 主机输入/从机输出
                         P2.7(SCLK) - 时钟信号*/

    // 配置USART2为SPI从模式
    USART2CR1 = 0x00;   // 先清除所有位
    USART2CR1 = (0 << 7) |   // LINEN=0 禁用lin
                (0 << 6) |   // DORD=0（MSB优先）高位优先
                (0 << 5) |   // CLKEN=0 禁用时钟
                (1 << 4) |   // SPMOD=1 使能SPI模式
                (1 << 3) |   // SPIEN=1 使能SPI功能
                (1 << 2) |   // SPSLV=1（从模式）
                (0 << 1) |   // CPOL=0 空闲时候低电平
                (0 << 0);    // CPHA=0 第一个边缘采样
    P_SW2 &= ~0x80;       // 恢复EAXFR=0    
    
    // 配置P2.3(SS)为输入模式
    P2M1 |= (1 << 3);
    P2M0 &= ~(1 << 3);

    // 配置P2.5(MOSI)为输入模式 - 接收主机数据
    P2M1 |= (1 << 5);
    P2M0 &= ~(1 << 5);

    // 配置P2.6(MISO)为推挽输出模式 - 向主机发送数据
    P2M1 &= ~(1 << 6);
    P2M0 |= (1 << 6);
    
    // 配置P2.7(SCLK)为输入模式 - 接收主机时钟信号
    P2M1 |= (1 << 7);
    P2M0 &= ~(1 << 7);
    
    // 配置定时器1作为USART2的时钟源
    TMOD &= ~0xF0;       // 清除T1相关设置
    TMOD |= 0x20;        // T1为8位自动重装模式
    AUXR &= ~(1 << 6);   // T1为1T模式
    TH1 = 0xFF;          // 设置定时器初值为最大值以获得最高波特率
    TL1 = 0xFF;          
    TR1 = 1;             // 启动定时器1
    
    // 配置USART2为同步模式
    S2CON = 0x10;        // S2SM0=0, S2SM1=0, S2REN=1: 同步移位寄存器模式且使能接收
    
    // 使能USART2中断
    IE2 |= 0x01;         // ES2=1: 使能串口2中断
    EA = 1;              // 总中断使能
    
    // 准备接收缓冲区，初始化发送数据为默认值
    S2BUF = 0xFF;
    
    spi_slave_mode_enabled = 1;
}

// 禁用SPI从模式
void SPI_DisableSlave(void) //留在这里huh
{
    // 关闭USART2
    S2CON = 0x00;        // 关闭USART2
    
    // 禁用T1时钟输出
    TR1 = 0;             // 停止定时器1
    
    // 禁用USART2中断
    IE2 &= ~0x01;        // 禁用串口2中断
    
    spi_slave_mode_enabled = 0;
}

// 设置SPI从模式接收回调函数
void SPI_SetSlaveRxCallback(void (*callback)(unsigned char))
{
    spi_slave_rx_callback = callback;
}

// 设置SPI从模式发送回调函数
void SPI_SetSlaveTxCallback(unsigned char (*callback)(void))
{
    spi_slave_tx_callback = callback;
}

// 准备要发送的数据（主机请求时将发送此数据）
void SPI_SlavePrepareTxData(unsigned char dataSend)
{
    S2BUF = dataSend;  // 直接写入要发送的数据到S2BUF寄存器
}

// USART2中断服务程序 - 用于处理从P2口接收到的SPI数据
void USART2_Isr() interrupt 8  // 使用中断8号 (UART2中断)
{
    if (S2CON & 0x01)  // S2RI: 接收中断标志
    {
        unsigned char received_data = S2BUF;
        S2CON &= ~0x01;  // 清除接收中断标志
        
        // 如果注册了接收回调函数，则调用
        if (spi_slave_rx_callback)
        {
            spi_slave_rx_callback(received_data);
        }
    }
    
    if (S2CON & 0x02)  // S2TI: 发送中断标志
    {
        S2CON &= ~0x02;  // 清除发送中断标志
        
        // 如果注册了发送回调函数，则准备下一个要发送的数据
        if (spi_slave_tx_callback)
        {
            S2BUF = spi_slave_tx_callback();
        }
        else
        {
            // 默认发送0xFF
            S2BUF = 0xFF;
        }
    }
}

#pragma endregion SPI从模式