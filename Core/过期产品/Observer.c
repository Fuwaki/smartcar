#include <STC32G.H>
#include "Muti_SPI_Device.h"
#include "Observer.h"

// 配置SPI数据缓冲区
#define SPI_BUFFER_SIZE 256 // 缓冲区大小，可以根据需要调整
// SENSOR_DATA sensor_data; // 声明全局变量，用于存储传感器数据    
SPI_Device *spi_dev;                              // 用于存储SPI设备指针
unsigned char spi_buffer[SPI_BUFFER_SIZE]; // SPI数据缓冲区
unsigned char buffer_write_index = 0;      // 写入指针
unsigned char buffer_read_index = 0;       // 读取指针
unsigned char buffer_count = 0;            // 缓冲区中的数据数量
bit swich = 0; // 用于标记是否需要中断SPI接收数据

// 初始化SPI和观察者
void InitObserver(void)
{
    // 注册SPI设备并保存返回的设备指针
    spi_dev = SPI_DeviceRegister(SPI_MODE0, SPI_SPEED_4, 0, 0); // 设备1
}

// 将一个字节写入缓冲区
void PushToBuffer(unsigned char dataPush)
{
    if (buffer_count < SPI_BUFFER_SIZE)
    {
        spi_buffer[buffer_write_index] = dataPush;
        buffer_write_index = (buffer_write_index + 1) % SPI_BUFFER_SIZE;
        buffer_count++;
    } 
}

// 从缓冲区读取一个字节
unsigned char PopFromBuffer(void)
{
    unsigned char dataBuffer = 0;

    if (buffer_count > 0)
    {
        dataBuffer = spi_buffer[buffer_read_index];
        buffer_read_index = (buffer_read_index + 1) % SPI_BUFFER_SIZE;
        buffer_count--;
    }

    return dataBuffer;
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
    unsigned char dataRead = 0;

    if (spi_dev)
    {
        // 选择SPI设备
        SPI_Select(spi_dev);
        dataRead = SPI_Read();
        SPI_Deselect(spi_dev);
        // 存储到缓冲区
        PushToBuffer(dataRead);
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
        SPI_Deselect(spi_dev);
    }
}

// 从SPI缓冲区解析fk到SENSOR_DATA结构体
void ParseSensorData(SENSOR_DATA* sensor_data)
{
    unsigned char* dst;
    unsigned int i;
    // 使用指针指向结构体首地址，便于按字节填充
    dst = (unsigned char*)sensor_data;
    swich = 0; // 解析完成后重置开关
    // Check if the buffer has enough data
    if (!sensor_data || GetBufferCount() < sizeof(SENSOR_DATA))
    {
        return; //FUCK U SPI!
    }

    // 逐字节从缓冲区读取数据并填入结构体
    for (i = 0; i < sizeof(SENSOR_DATA); i++)
    {
        *dst++ = PopFromBuffer();
    }

    swich = 1;//解析完成
}

// 丢弃指定数量的数据
void DiscardData(unsigned char count)
{
    unsigned char i;
    for(i = 0; i < count && buffer_count > 0; i++)
    {
        PopFromBuffer();
    }
}

short UpdateSensorData(void)
{
    if (swich = 1)
        ReadSPIToBuffer();

    if (GetBufferCount() >= sizeof(SENSOR_DATA))
    {
        // 解析数据
        ParseSensorData(&sensor_data);
        return 1; // 成功更新
    }

    if (GetBufferCount() > sizeof(SENSOR_DATA)* 3/2) //出问题了qwq
    {
        DiscardData(GetBufferCount());
        return -1;
    }
    
    return 0; // 未能更新
}