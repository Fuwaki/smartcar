#ifndef _MUTI_SPI_DEVICE_H_
#define _MUTI_SPI_DEVICE_H_
    //这个只需要主模式就可以了，因为是核心板
    #include <STC32G.H>

    // SPI模式定义
    /*CPOL（Clock Polarity）：定义时钟空闲时的电平。STC32G12K128 支持两种状态：
    CPOL=0：空闲时 SCLK 为低电平。
    CPOL=1：空闲时 SCLK 为高电平。
    CPHA（Clock Phase）：定义数据采样的时钟边沿：
    CPHA=0：在第一个时钟边沿（上升沿或下降沿，取决于 CPOL）采样。
    CPHA=1：在第二个时钟边沿采样。*/
    #define SPI_MODE0   0x00    // CPOL=0, CPHA=0 空闲低电平，第一个边沿采样
    #define SPI_MODE1   0x04    // CPOL=0, CPHA=1 空闲低电平，第二个边沿采样
    #define SPI_MODE2   0x08    // CPOL=1, CPHA=0 空闲高电平，第一个边沿采样
    #define SPI_MODE3   0x0C    // CPOL=1, CPHA=1 空闲高电平，第二个边沿采样

    // SPI传输速率预分频值
    typedef enum {
        SPI_SPEED_4   = 0x00,   // 系统时钟/4
        SPI_SPEED_8   = 0x01,   // 系统时钟/8
        SPI_SPEED_16  = 0x02,   // 系统时钟/16
        SPI_SPEED_32  = 0x03    // 系统时钟/32
    } SPI_Speed_TypeDef;

    // SPI设备定义
    typedef struct {
        unsigned char mode;           // SPI模式 (SPI_MODE0/1/2/3)
        SPI_Speed_TypeDef speed;      // SPI速度 qwq
        unsigned char cs_pin;         // 片选引脚号
        unsigned char cs_port;        // 片选端口号 (0-7对应P0-P7)
    } SPI_Device;

    // 初始化SPI总线
    void SPI_Init(void);

    // 配置SPI通信参数
    void SPI_Config(unsigned char mode, SPI_Speed_TypeDef speed);

    // 选择SPI设备
    void SPI_Select(SPI_Device *device);

    // 取消选择SPI设备
    void SPI_Deselect(SPI_Device *device);

    // 发送并接收一个字节的数据
    unsigned char SPI_ReadWrite(unsigned char dat);

    // 只发送一个字节数据
    void SPI_Write(unsigned char dat);

    // 只读取一个字节数据
    unsigned char SPI_Read(void);

    // 发送多个字节
    void SPI_WriteBuffer(unsigned char *pBuf, unsigned int len);

    // 读取多个字节
    void SPI_ReadBuffer(unsigned char *pBuf, unsigned int len);

    // 注册SPI设备
    SPI_Device* SPI_DeviceRegister(unsigned char mode, SPI_Speed_TypeDef speed, unsigned char cs_port, unsigned char cs_pin);

#endif /* _MUTI_SPI_DEVICE_H_ */
