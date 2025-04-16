#include <AI8051U.H>
#include <intrins.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "Timer.h"
#include "uart.h"
#include "GPS_uart.h"
#include "GPS.h"
#include "Encoder.h"

#include "SPI_MultiDevice.h"
#include "Gyroscope.h"
#include "Magnetic.h"

//先写个这样的函数丢这
// void SPI_SEND(char *str){

// }
// void Construct_Data_Frame(struct Posture p){
// 	//要和核心板的数据接受的解析代码相匹配
// 	char *s = "";
// 	SPI_SEND(s);
// }

unsigned char j;

float datatosend[19] = {0.0};

unsigned char receive_buffer0d00[32];
unsigned char receive_buffer0d01[32];

void Delay100us(void)	//@40.000MHz
{
	unsigned long edata i;

	_nop_();
	_nop_();
	_nop_();
	i = 998UL;
	while (i) i--;
}


void Delay1000ms(void)	//@40.000MHz
{
	unsigned long edata i;

	_nop_();
	_nop_();
	i = 9999998UL;
	while (i) i--;
}


void Inits()
{
	EAXFR = 1;
	CKCON = 0x00;
	WTST = 0x00;

	P0M1 = 0x00;P0M0 = 0x00;
	P1M1 = 0x00;P1M0 = 0x00;
    P2M0 = 0x00;P2M1 = 0x00; //为SPI从模式 P2.6为输入模式
	P3M1 = 0x00;P3M0 = 0x00;
	P4M1 = 0x00;P4M0 = 0x00;
	P5M1 = 0x00;P5M0 = 0x00;
	P6M1 = 0x00;P6M0 = 0x00;
	P7M1 = 0x00;P7M0 = 0x00;
	
	Timer0_Init(); //定时器0初始化
	SPI_Init(); //SPI初始化
	UART_Init(); //串口初始化
	GPS_UART_Init(); //GPS串口初始化
	Init_GPS_Setting(); //GPS初始化
	Encoder_Init(); //编码器初始化
	// SPI_InitSlave(); //SPI从模式初始化
	ICM42688_Init(); //陀螺仪初始化
	LIS3MDL_Init();

}

void messageUpdaterWithUart()
{
	if (rmc_data.valid)
	{
		#pragma region GPS数据
		datatosend[0] = rmc_data.latitude_decimal; // 纬度数据
		datatosend[1] = rmc_data.longitude_decimal;
		datatosend[2] = naturePosition.x; // 纬度数据
		datatosend[3] = naturePosition.y; // 经度数据
		datatosend[4] = rmc_data.course; // 航向数据
		datatosend[5] = rmc_data.speed; // 速度数据
	}
	else
	{
		datatosend[0] = 0.0; // 纬度数据
		datatosend[1] = 0.0; // 经度数据
		datatosend[2] = 0.0; // 纬度数据
		datatosend[3] = 0.0; // 经度数据
		datatosend[4] = 0.0; // 航向数据
		datatosend[5] = 0.0; // 速度数据
		#pragma endregion GPS数据
	}

	#pragma region 陀螺仪
	datatosend[6] = gyro_data.accel_x_g;
	datatosend[7] = gyro_data.accel_y_g;
	datatosend[8] = gyro_data.accel_z_g;
	datatosend[9] = gyro_data.gyro_x_dps;
	datatosend[10] = gyro_data.gyro_y_dps;
	datatosend[11] = gyro_data.gyro_z_dps;
	datatosend[12] = gyro_data.temp; // 温度数据
	datatosend[13] = gyro_data.true_yaw_angle; // 真航向角数据
	#pragma endregion 陀螺仪

	#pragma region 磁场计数据
	datatosend[14] = mag_data.x_gauss;
	datatosend[15] = mag_data.y_gauss;
	datatosend[16] = mag_data.z_gauss;
	datatosend[17] = mag_data.heading;
	#pragma endregion 磁场计数据


	#pragma region 编码器数据
	datatosend[18] = encoder.position; // 速度数据
	#pragma endregion 编码器数据
	
}

void main()
{
	float baba = 0;
	unsigned char GPS_Init = 1;
	Inits();
	Delay100us();
	// UART_SendByte('S'); //Started!
	while(1)
	{
		yaw_angle_init(); //航向角初始化
		Encoder_Update(); //更新编码器数据
		messageUpdaterWithUart(); //更新数据
		UART_SendFloat(datatosend); //发送数据
		//// SPI_SlaveModeMessageUpdater(&senddata); //更新数据
		// // // 只有在上一次传输完成后才启动新的传输
		//// if(!SPI_IsStructTransmissionActive())
		//// {
		//// 	SPI_SlaveStartSendSensorData(&senddata); //开始发送数据
		//// }
		
		#pragma region GPS数据模块
		GPS_Message_Updater();
		if (GPS_Init == 1)
		{
			GPS_Init = Init_GPS_Offset(&naturePosition, &rmc_data);
			if (GPS_Init == 0)
			{
				UART_SendStr("GPS偏移量初始化成功!\0");
				GPS_Init = 0;
			}
		}
		#pragma endregion
		
		#pragma region 陀螺仪//0d00
		Gyro_Updater();
		#pragma endregion
		// #pragma region 编码器数据模块
		// Encoder_Update();
		// #pragma endregion
		
		
		#pragma region 磁场计数据模块
		baba = LIS3MDL_ReadData(&mag_data); // 读取磁力计数据
		#pragma endregion

		// a = (float) mag_data.x_mag;
		// b = (float) mag_data.y_mag;
		// c = (float) mag_data.z_mag;
		// value[0] = a;
		// value[1] = b;
		// value[2] = baba;
		// UART_SendFloat(value);


		// // UART_SendByte(((unsigned char*)(&mag_data.x_mag))[0]);
		// // UART_SendByte(((unsigned char*)(&mag_data.x_mag))[1]);

		// // UART_SendStr("数据读取完成!\0");
		// if(UART_Available()) // 	这个是接收数据的函数
		// {
		// 	unsigned char received_len;
		// 	received_len = UART_Read(receive_buffer0d00, 32);
		// 	if(received_len > 0)
		// 	{
		// 		receive_buffer0d00[received_len] = '\0';  // 添加字符串结束符
		// 		// 通过串口发送接收到的数据
		// 		UART_SendStr(receive_buffer0d00);
		// 	}
		// }
	}
}