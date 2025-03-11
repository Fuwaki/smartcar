#include "SPI_MultiDevice.h"
#include <stdio.h>

void SPI_MultiDevice_Example(void)
{
    // 初始化SPI主机
    SPI_Init();

    // 配置两个SPI从设备
    spi_slave_config_t slave1_config = {
        .cs_port = 3,      // P3口
        .cs_pin = 2,       // P3.2
        .mode = SPI_MODE0, // 模式0 (CPOL=0, CPHA=0)
        .clock_div = 0     // 时钟分频 (取最快)
    };

    spi_slave_config_t slave2_config = {
        .cs_port = 3,      // P3口
        .cs_pin = 3,       // P3.3
        .mode = SPI_MODE3, // 模式3 (CPOL=1, CPHA=1)
        .clock_div = 2     // 时钟分频 (/8)
    };

    // 注册两个SPI从设备
    unsigned char slave1_id = SPI_RegisterSlave(&slave1_config);
    unsigned char slave2_id = SPI_RegisterSlave(&slave2_config);

    // 读取设备1的ID寄存器
    unsigned char device1_id = SPI_ReadRegister(slave1_id, 0x00);
    printf("Device 1 ID: 0x%02X\n", device1_id);

    // 写入设备2的配置寄存器
    SPI_WriteRegister(slave2_id, 0x10, 0xA5);

    // 从设备1读取多个寄存器
    unsigned char buffer[10];
    SPI_ReadMultiRegisters(slave1_id, 0x20, buffer, 10);

    printf("Device 1 Registers:\n");
    for (unsigned char i = 0; i < 10; i++)
    {
        printf("Reg[0x%02X] = 0x%02X\n", 0x20 + i, buffer[i]);
    }

    // 向设备2写入多个寄存器
    for (unsigned char i = 0; i < 10; i++)
    {
        buffer[i] = i * 10;
    }
    SPI_WriteMultiRegisters(slave2_id, 0x30, buffer, 10);
}
