#include <STC32G.H>
#include "Muti_SPI_Device.h"
#include "Observer.h"

// 配置SPI数据缓冲区
#define SPI_BUFFER_SIZE 64 // 缓冲区大小，可以根据需要调整

SPI_Device *spi_dev;                              // 用于存储SPI设备指针
static unsigned char spi_buffer[SPI_BUFFER_SIZE]; // SPI数据缓冲区
static unsigned char buffer_write_index = 0;      // 写入指针
static unsigned char buffer_read_index = 0;       // 读取指针
static unsigned char buffer_count = 0;            // 缓冲区中的数据数量

// 初始化SPI和观察者
void InitObserver(void)
{
    // 注册SPI设备并保存返回的设备指针
    spi_dev = SPI_DeviceRegister(SPI_MODE0, SPI_SPEED_4, 0, 0); // 设备1
}

// 将一个字节写入缓冲区
static void PushToBuffer(unsigned char data)
{
    if (buffer_count < SPI_BUFFER_SIZE)
    {
        // 缓冲区未满，可以写入
        spi_buffer[buffer_write_index] = data;
        buffer_write_index = (buffer_write_index + 1) % SPI_BUFFER_SIZE; // 循环缓冲区
        buffer_count++;
    }
    // 如果缓冲区满，则新数据会被丢弃
}

// 从缓冲区读取一个字节
unsigned char PopFromBuffer(void)
{
    unsigned char data = 0;

    if (buffer_count > 0)
    {
        // 有数据可读
        data = spi_buffer[buffer_read_index];
        buffer_read_index = (buffer_read_index + 1) % SPI_BUFFER_SIZE; // 循环缓冲区
        buffer_count--;
    }

    return data;
}

// 检查缓冲区是否有数据可读
bit IsDataAvailable(void)
{
    return (buffer_count > 0);
}

// 获取缓冲区中数据的数量
unsigned char GetBufferCount(void)
{
    return buffer_count;
}

// 从SPI读取一个字节并存储到缓冲区
void ReadSPIToBuffer(void)
{
    unsigned char data = 0;

    if (spi_dev)
    { // 确保SPI设备已成功注册
        // 选择SPI设备
        SPI_Select(spi_dev);

        // 读取数据
        data = SPI_Read();

        // 释放SPI设备
        SPI_Release(spi_dev);

        // 存储到缓冲区
        PushToBuffer(data);
    }
}

// 连续读取多个字节到缓冲区
void ReadMultiSPIToBuffer(unsigned char count)
{
    unsigned char i;

    if (spi_dev)
    { // 确保SPI设备已成功注册
        // 选择SPI设备
        SPI_Select(spi_dev);

        // 读取多个数据
        for (i = 0; i < count; i++)
        {
            PushToBuffer(SPI_Read());
        }

        // 释放SPI设备
        SPI_Release(spi_dev);
    }
}