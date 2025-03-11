#include "SPI_MultiDevice.h"
#include <intrins.h>
#include <stdio.h>


// 存储已注册的SPI从设备配置
static spi_slave_config_t spi_slaves[MAX_SPI_SLAVES];
static unsigned char slave_count = 0;
static unsigned char current_slave = 0xFF; // 没有选中的从设备

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
    SPCTL = 0x50;  // 设置为主模式, SSIG=1(忽略SS引脚), SPEN=1(启用SPI)
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
