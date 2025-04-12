#ifndef __OBSERVER_H__
#define __OBSERVER_H__

#include <STC32G.H>
#include "Muti_SPI_Device.h"

    // typedef struct 
    // {
    //     #pragma region GPS数据
    //     float GPS_Raw_X;//纬度
    //     float GPS_Raw_Y;//经度
    //     float GPS_Nature_X;
    //     float GPS_Nature_Y;
    //     float GPS_Heading;
    //     float GPS_Speed;
    //     #pragma endregion GPS数据

    //     #pragma region IMU数据
    //     float IMU_Acc_X;
    //     float IMU_Acc_Y;
    //     float IMU_Acc_Z;
    //     float IMU_Gyro_X; //dps
    //     float IMU_Gyro_Y;
    //     float IMU_Gyro_Z;
    //     float IMU_Temperature;
    //     #pragma endregion IMU数据

    //     #pragma region Mag数据
    //     float Mag_Adujsted_X;
    //     float Mag_Adujsted_Y;
    //     float Mag_Adujsted_Z;
    //     float Mag_Heading;
    //     #pragma endregion Mag数据

    //     #pragma region 编码器数据
    //     float Encoder_Speed;
    //     #pragma endregion 编码器数据
    // }SENSOR_DATA;

    // extern SENSOR_DATA sensor_data; // 声明全局变量

    void InitObserver(void);
    unsigned char PopFromBuffer(void);
    bit IsDataAvailable(void);
    unsigned char GetBufferCount(void);
    void ReadSPIToBuffer(void);
    void PushToBuffer(unsigned char dataPush);
    void ReadMultiSPIToBuffer(unsigned char count);
    void ParseSensorData(SENSOR_DATA* sensor_data);
    
    // 新增的缓冲区管理和数据更新函数
    void DiscardData(unsigned char count);
    short UpdateSensorData(void);
#endif