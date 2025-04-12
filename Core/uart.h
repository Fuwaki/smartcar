#ifndef __UART_H__
#define __UART_H__
    typedef struct 
    {
        #pragma region GPS数据
        float GPS_Raw_X;//纬度
        float GPS_Raw_Y;//经度
        float GPS_Nature_X;
        float GPS_Nature_Y;
        float GPS_Heading;
        float GPS_Speed;
        #pragma endregion GPS数据

        #pragma region IMU数据
        float IMU_Acc_X;
        float IMU_Acc_Y;
        float IMU_Acc_Z;
        float IMU_Gyro_X; //dps
        float IMU_Gyro_Y;
        float IMU_Gyro_Z;
        float IMU_Temperature;
        #pragma endregion IMU数据

        #pragma region Mag数据
        float Mag_Adujsted_X;
        float Mag_Adujsted_Y;
        float Mag_Adujsted_Z;
        float Mag_Heading;
        #pragma endregion Mag数据

        #pragma region 编码器数据
        float Encoder_Speed;
        #pragma endregion 编码器数据
    }SENSOR_DATA;
    extern SENSOR_DATA sensor_data; // 声明全局变量h

    // 串口初始化
    void Uart3Init();
    void Uart1Init();
    // 发送单个字节
    void Uart3Send(char dat);
    void UartSend(char dat);
    // 发送字符串
    void UartSendStr(char *p);
    void Uart3SendStr(char *p);
    // 发送数据
    void Uart3SendByLength(unsigned char *p,int length);
    void UartSendByLength(unsigned char *p,int length);

    void Uart3CheckAndReceive(void);

    void UartReceiveSensorData(void);
    void UART_SendFloat(float value[18]); // 发送浮点数数据
#endif