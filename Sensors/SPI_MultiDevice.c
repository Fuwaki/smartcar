#include "SPI_MultiDevice.h"
#include <AI8051U.H>
#include <intrins.h>
#include <stdio.h>
#include "Magnetic.h"
#include "Gyroscope.h"
#include "Encoder.h"
#include "GPS.h"

// 存储已注册的SPI从设备配置
spi_slave_config_t spi_slaves[MAX_SPI_SLAVES];
unsigned char slave_count = 0;
unsigned char current_slave = 0xFF; // 没有选中的从设备

// 浮点数传输相关变量和函数
unsigned char float_tx_buffer[4]; // 浮点数传输缓冲区
unsigned char float_tx_index = 0; // 当前发送字节索引

SENSOR_DATA* sensor_data_to_send = NULL; // 指向要发送的结构体
unsigned char* sensor_data_ptr = NULL;   // 用于按字节访问结构体
unsigned int sensor_data_size = 0;       // 结构体总大小
unsigned int sensor_data_index = 0;      // 当前发送的字节索引
bit sensor_data_tx_active = 0;           // 结构体传输活动标志

SENSOR_DATA senddata; // 结构体实例，用于存储传感器数据

// SPI从模式回调函数指针
void (*spi_slave_rx_callback)(unsigned char) = NULL;
unsigned char (*spi_slave_tx_callback)(void) = NULL;

// SPI从模式状态
bit spi_slave_mode_enabled = 0; //0d00
bit spi_master_rx = 0; // SPI接收标志位
bit spi_master_tx = 0; // SPI发送标志位
bit float_tx_active = 0; // 浮点数传输活动标志

sbit S1MISO = P2^5; // SPI主设备输入引脚
sbit S1SCLK = P2^7; // SPI时钟引脚

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
    SPCTL = 0xD0;  // 设置为主模式, SSIG=0(SS引脚有效), SPEN=1(启用SPI)
                   // 0x50 = 11010000b：
                   // 位7(SSIG)=1：ss引脚无效（忽略SS引脚）
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
unsigned char SPI_TransferByte(unsigned char data_out) //?这里是否有问题？
{
    while (spi_master_tx); // 等待之前的发送完成
    spi_master_tx = 1; // 设置发送标志位
    SPDAT = data_out; // 写入数据启动传输
    
    // 等待SPI传输完成 (检查SPIF位)
    while (!(SPSTAT & 0x80)); // 等待SPIF标志位置1
    SPSTAT = 0xC0; // 清除写冲突和SPI完成标志
    
    spi_master_tx = 0; // 清除发送标志位
    return SPDAT; // 返回接收到的数据
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

// SPI从模式初始化函数 - 使用USART2和定时器2实现SPI从模式
void SPI_InitSlave(void)
{
    // 开启EAXFR访问权限
    P_SW2 |= 0x80;     
    // 设置USART2的SPI功能映射到P2口
    P_SW3 = (P_SW3 & ~0x30) | 0x10;	//USART2_SPI: S2SS(P2.4), S2MOSI(P2.5), S2MISO(P2.6), S2SCLK(P2.7)
    
    // 配置USART2工作模式和接收使能
    S2CON = 0x50;      // 0x10 = 00010000b:
                      // bit7-5: 000 - 采用P_SW3选择引脚映射
                      // bit4: 1 - 允许接收
                      // bit3-0: 0 - 相关控制位清0

    // 配置SPI从机的引脚模式
    // // P2.4(SS)为输入模式 - 必须是上拉输入
    // P2M1 |= (1 << 4);   // 设为1
    // P2M0 &= ~(1 << 4);  // 设为0 -> 高阻输入
    // P2PU |= (1 << 4);   // 开启上拉电阻

    // P2.5(MOSI)为输入模式 - 必须是上拉输入
    P2M1 |= (1 << 5);   // 设为1
    P2M0 &= ~(1 << 5);  // 设为0 -> 高阻输入
    P2PU |= (1 << 5);   // 开启上拉电阻
    P2NCS |= (1 << 5);  // 开启施密特触发

    // P2.6(MISO)为推挽输出 - 作为从机的输出
    P2M1 &= ~(1 << 6);  // 设为0
    P2M0 |= (1 << 6);   // 设为1 -> 推挽输出
    P2SR &= ~(1 << 6);  // 设置为快速翻转模式

    // P2.7(SCLK)为输入模式 - 必须是上拉输入
    P2M1 |= (1 << 7);   // 设为1
    P2M0 &= ~(1 << 7);  // 设为0 -> 高阻输入
    P2PU |= (1 << 7);   // 开启上拉电阻
    P2NCS |= (1 << 7);  // 开启施密特触发
    
    // 配置USART2为SPI从模式
    USART2CR1 = 0x1C;   // 0x1C = 00011100b:
                        // bit7-6: 保留位
                        // bit5: 0 - 禁用时钟
                        // bit4: 1 - 使能SPI模式
                        // bit3: 1 - 使能SPI功能
                        // bit2: 1 - 从模式
                        // bit1: 1 - CPOL=1空闲时时钟为高
                        // bit0: 0 - CPHA=0第一个时钟边沿采样
    USART2CR4 = 0x00;                   //SPI速度为SYSCLK/4
    USART2CR1 |= 0x08;                  //使能SPI功能
    // 配置定时器2为USART2的波特率发生器
    AUXR |= 0x04;      // T2为1T模式
    T2L = 0xFC;        // 设置定时器2初值为0xFC，定时器2计数到0x00时产生中断
    T2H = 0xFF;
    
    // 启动定时器2
    AUXR |= 0x10;

    // 清除可能的中断标志
    S2TI = 0;
    S2RI = 0;
    
    // 使能USART2中断
    // IE2 |= 0x01;       // ES2=1: 使能串口2中断
    ES2 = 1;         // 使能USART2接收中断
    EA = 1;            // 总中断使能
    
    // 初始化发送缓冲区，准备好第一个默认值
    S2BUF = 0xFF;
    
    spi_slave_mode_enabled = 1;
}

// 禁用SPI从模式
void SPI_DisableSlave(void) //留在这里huh
{
    // 关闭USART2
    S2CON = 0x00;        // 关闭USART2
    
    // // 禁用T2时钟输出
    // AUXR &= ~0x10;       // 停止定时器2
    
    // 禁用USART2中断
    ES2 = 0; //  禁用USART2接收中断
    
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
void USART2_Isr() interrupt UART2_VECTOR  // 使用中断8号 (UART2中断)
{
    unsigned char received_data;
    if (S2RI)  // S2RI: 接收中断标志
    {
        received_data = S2BUF;
        S2RI = 0;  // 清除接收中断标志

        
        // 如果注册了接收回调函数，则调用
        if (spi_slave_rx_callback)
        {
            spi_slave_rx_callback(received_data); // 调用接收回调函数
        }
    }
    
    if (S2TI)  // S2TI: 发送中断标志
    {
        S2TI = 0;  // 清除发送中断标志
        
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

// 开始发送SENSOR_DATA结构体数据的函数
void SPI_SlaveStartSendSensorData(SENSOR_DATA* connectData)
{
    if (!spi_slave_mode_enabled || !connectData)
        return; //FKU SPI!
    
    // 设置要发送的结构体指针
    sensor_data_to_send = connectData;
    // 计算结构体大小
    sensor_data_size = sizeof(SENSOR_DATA);
    // 重置发送索引
    sensor_data_index = 0;
    // 获取结构体字节指针，用于按字节访问
    sensor_data_ptr = (unsigned char*)connectData;
    // 设置传输活动标志
    sensor_data_tx_active = 1;
    
    // 注册发送回调函数
    SPI_SetSlaveTxCallback(SPI_SlaveSendSensorDataByte);
    
    // 准备第一个字节发送
    SPI_SlavePrepareTxData(sensor_data_ptr[0]);
}

// 结构体数据发送回调函数 - 每次主机读取一个字节时调用
unsigned char SPI_SlaveSendSensorDataByte(void)
{
    unsigned char data_byte;
    
    if (!sensor_data_tx_active || !sensor_data_to_send)
    {
        return 0xFF; // 默认返回0xFF
    }
    
    // 获取当前要发送的字节
    data_byte = sensor_data_ptr[sensor_data_index];
    
    // 增加索引，准备下一个字节
    sensor_data_index++;
    
    // 检查是否发送完毕
    if (sensor_data_index >= sensor_data_size) //restart?
    {
        sensor_data_tx_active = 0;
        sensor_data_to_send = NULL;
        sensor_data_ptr = NULL;
        sensor_data_index = 0;
        
        // SPI_SetSlaveTxCallback(NULL); //如果你想要的话
    }
    
    return data_byte;
}

// 检查结构体传输是否活动
bit SPI_IsStructTransmissionActive(void)
{
    return sensor_data_tx_active;
}

// 取消当前的结构体传输
void SPI_CancelStructTransmission(void)  //DEBUG使用
{
    EA = 0; // 禁用中断
    
    // 重置所有传输相关变量
    sensor_data_tx_active = 0;
    sensor_data_to_send = NULL;
    sensor_data_ptr = NULL;
    sensor_data_index = 0;
    
    // 恢复默认回调
    SPI_SetSlaveTxCallback(NULL);
    
    EA = 1; // 恢复中断
}

#pragma endregion SPI从模式

void SPI_SlaveModeMessageUpdater(SENSOR_DATA* connectData)
{
    //我决定在这里更新数据!
    // 这里可以添加代码来更新connectData中的数据
    if (spi_slave_mode_enabled == 1)
    {
        #pragma region 陀螺仪
        connectData->IMU_Acc_X = gyro_data.accel_x;
        connectData->IMU_Acc_Y = gyro_data.accel_y;
        connectData->IMU_Acc_Z = gyro_data.accel_z;
        connectData->IMU_Temperature = gyro_data.temp; // 温度数据
        connectData->IMU_Gyro_X = gyro_data.gyro_x;
        connectData->IMU_Gyro_Y = gyro_data.gyro_y;
        connectData->IMU_Gyro_Z = gyro_data.gyro_z;
        #pragma endregion 陀螺仪

        #pragma region GPS数据
        connectData->GPS_Raw_X = rmc_data.latitude; // 纬度数据
        connectData->GPS_Raw_Y = rmc_data.longitude; // 经度数据
        connectData->GPS_Nature_X = naturePosition.x; // 纬度数据
        connectData->GPS_Nature_Y = naturePosition.y; // 经度数据
        connectData->GPS_Heading = rmc_data.course; // 航向数据
        connectData->GPS_Speed = rmc_data.speed; // 速度数据
        #pragma endregion GPS数据

        #pragma region 编码器数据
        // connectData->Encoder_Speed = encoder_data.speed; // 速度数据
        #pragma endregion 编码器数据
        
        #pragma region 磁场计数据
        connectData->Mag_Adujsted_X = mag_data.x_mag; //改
        connectData->Mag_Adujsted_Y = mag_data.y_mag; //改
        connectData->Mag_Adujsted_Z = mag_data.z_mag; //改
        connectData->Mag_Heading = mag_data.heading;
        #pragma endregion 磁场计数据
    }
    else
    {
        // SPI从模式未启用,为节省资源,不更新数据
        return;
    }
}