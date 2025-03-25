#include "Muti_SPI_Device.h"
#include <stdio.h>
// SPI设备列表
static SPI_Device spi_devices[8];           // 最多支持8个设备
static unsigned char device_count = 0;      // 当前注册的设备数量
static unsigned char current_device = 0xFF; // 当前选中的设备, 0xFF表示没有选中设备

// 定义CS引脚控制函数
void CS_Control(SPI_Device *device, unsigned char state)
{
    // 访问扩展SFR
    P_SW2 |= 0x80; // 使能访问XSFR

    switch (device->cs_port)
    {
    case 0:
        if (state)
            P0 |= (1 << device->cs_pin);
        else
            P0 &= ~(1 << device->cs_pin);
        break;
    case 1:
        if (state)
            P1 |= (1 << device->cs_pin);
        else
            P1 &= ~(1 << device->cs_pin);
        break;
    case 2:
        if (state)
            P2 |= (1 << device->cs_pin);
        else
            P2 &= ~(1 << device->cs_pin);
        break;
    case 3:
        if (state)
            P3 |= (1 << device->cs_pin);
        else
            P3 &= ~(1 << device->cs_pin);
        break;
    case 4:
        if (state)
            P4 |= (1 << device->cs_pin);
        else
            P4 &= ~(1 << device->cs_pin);
        break;
    case 5:
        if (state)
            P5 |= (1 << device->cs_pin);
        else
            P5 &= ~(1 << device->cs_pin);
        break;
    case 6:
        if (state)
            P6 |= (1 << device->cs_pin);
        else
            P6 &= ~(1 << device->cs_pin);
        break;
    case 7:
        if (state)
            P7 |= (1 << device->cs_pin);
        else
            P7 &= ~(1 << device->cs_pin);
        break;
    }

    // 禁用访问XSFR
    P_SW2 &= ~0x80;
}

// 初始化SPI总线
void SPI_Init(void)
{
    unsigned char i;

    // 设置SPI引脚 (根据实际需求修改)
    P_SW2 |= 0x80; // 使能访问XSFR
    
    // 设置为推挽输出和输入
    P1M0 |= 0x2C;  // P1.2/P1.3/P1.5设为推挽输出(SS/MOSI/SCLK) -> 00101100b
    P1M1 &= ~0x2C; // 清除P1.2/P1.3/P1.5的M1位
    
    P1M0 &= ~0x10; // P1.4(MISO)设为输入 -> 00010000b
    P1M1 |= 0x10;  // 设置P1.4的M1位
    
    P_SW2 &= ~0x80; // 禁用访问XSFR QwQ
    
    /*SSIG (位7)：SS引脚忽略位，1表示忽略SS引脚
    SPEN (位6)：SPI使能位，1表示启用SPI功能
    DORD (位5)：数据顺序位，未设置为0
    MSTR (位4)：主/从模式选择位，1表示主机模式
    CPOL (位3)：时钟极性位，未设置为0
    CPHA (位2)：时钟相位位，未设置为0
    SPR1 (位1)：波特率控制位1，未设置为0
    SPR0 (位0)：波特率控制位0，未设置为0*/
    // 启用SPI功能，设为主机模式，SSIG=1(忽略SS引脚)
    SPCTL = (1 << 7) | (1 << 6) | (1 << 4); // SSIG=1, SPEN=1, MSTR=1

    // 清除状态标志
    SPSTAT = 0xC0; // 清除SPIF和WCOL标志

    // 默认配置 (Mode0, CLK/4)
    SPI_Config(SPI_MODE0, SPI_SPEED_4);//idk what is the speed

    // 初始化时将所有从设备的CS引脚设为高电平（未选中状态）
    for (i = 0; i < device_count; i++)
    {
        CS_Control(&spi_devices[i], 1); // 设置为高电平
    }
}

// 配置SPI通信参数
void SPI_Config(unsigned char mode, SPI_Speed_TypeDef speed)
{
    unsigned char tmp = SPCTL;

    // 保留高4位(SSIG, SPEN, DORD, MSTR)，清除低4位(CPOL, CPHA和SPR位)
    tmp &= 0xF0;

    // 设置新的模式和速度
    tmp |= mode;
    tmp |= speed;

    SPCTL = tmp;
}

// 选择SPI设备
void SPI_Select(SPI_Device *device)
{
    unsigned char i;
    // 如果当前有选中的设备，先取消选择
    if (current_device != 0xFF && current_device < device_count)
    {
        SPI_Deselect(&spi_devices[current_device]);
    }

    // 配置当前设备的SPI模式和速度
    SPI_Config(device->mode, device->speed);

    // 拉低CS引脚
    CS_Control(device, 0);

    // 找出当前设备在数组中的索引
    for (i = 0; i < device_count; i++)
    {
        if (&spi_devices[i] == device)
        {
            current_device = i;
            break;
        }
    }
}

// 取消选择SPI设备
void SPI_Deselect(SPI_Device *device)
{
    unsigned char i;
    // 拉高CS引脚
    CS_Control(device, 1);

    // 如果取消选择的是当前设备，重置current_device
    for (i = 0; i < device_count; i++)
    {
        if (&spi_devices[i] == device && current_device == i)
        {
            current_device = 0xFF;
            break;
        }
    }
}

// 发送并接收一个字节的数据
unsigned char SPI_ReadWrite(unsigned char dat)
{
    SPDAT = dat; // 发送数据
    while (!(SPSTAT & 0x80))
        ;          // 等待传输完成
    SPSTAT = 0xC0; // 清除中断标志和写冲突标志
    return SPDAT;  // 返回接收到的数据
}

// 只发送一个字节数据
void SPI_Write(unsigned char dat)
{
    SPI_ReadWrite(dat); // 发送数据并忽略接收数据
}

// 只读取一个字节数据
unsigned char SPI_Read(void)
{
    return SPI_ReadWrite(0xFF); // 发送0xFF(空闲状态)并读取数据
}

// 发送多个字节
void SPI_WriteBuffer(unsigned char *pBuf, unsigned int len)
{
    unsigned int i;
    for (i = 0; i < len; i++)
    {
        SPI_Write(pBuf[i]);
    }
}

// 读取多个字节
void SPI_ReadBuffer(unsigned char *pBuf, unsigned int len)
{
    unsigned int i;
    for (i = 0; i < len; i++)
    {
        pBuf[i] = SPI_Read();
    }
}

// 注册SPI设备
SPI_Device *SPI_DeviceRegister(unsigned char mode, SPI_Speed_TypeDef speed, unsigned char cs_port, unsigned char cs_pin)
{
    SPI_Device *device = NULL;

    if (device_count < 8)
    {
        device = &spi_devices[device_count++];
        device->mode = mode;
        device->speed = speed;
        device->cs_port = cs_port;
        device->cs_pin = cs_pin;

        // 初始化CS引脚为输出，默认高电平
        P_SW2 |= 0x80; // 使能访问XSFR

        switch (cs_port)
        {
        case 0:
            P0M0 |= (1 << cs_pin); // 设置为推挽输出
            P0M1 &= ~(1 << cs_pin);
            P0 |= (1 << cs_pin); // 默认高电平
            break;
        case 1:
            P1M0 |= (1 << cs_pin);
            P1M1 &= ~(1 << cs_pin);
            P1 |= (1 << cs_pin);
            break;
        case 2:
            P2M0 |= (1 << cs_pin);
            P2M1 &= ~(1 << cs_pin);
            P2 |= (1 << cs_pin);
            break;
        case 3:
            P3M0 |= (1 << cs_pin);
            P3M1 &= ~(1 << cs_pin);
            P3 |= (1 << cs_pin);
            break;
        case 4:
            P4M0 |= (1 << cs_pin);
            P4M1 &= ~(1 << cs_pin);
            P4 |= (1 << cs_pin);
            break;
        case 5:
            P5M0 |= (1 << cs_pin);
            P5M1 &= ~(1 << cs_pin);
            P5 |= (1 << cs_pin);
            break;
        case 6:
            P6M0 |= (1 << cs_pin);
            P6M1 &= ~(1 << cs_pin);
            P6 |= (1 << cs_pin);
            break;
        case 7:
            P7M0 |= (1 << cs_pin);
            P7M1 &= ~(1 << cs_pin);
            P7 |= (1 << cs_pin);
            break;
        }

        P_SW2 &= ~0x80; // 禁用访问XSFR
    }

    return device;
}
