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



float a, b, c;
float value[3] = {1.0, 0.0, 0.0};


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
	P2M1 = 0x00;P2M0 = 0x00;
	P3M1 = 0x00;P3M0 = 0x00;
	P4M1 = 0x00;P4M0 = 0x00;
	P5M1 = 0x00;P5M0 = 0x00;
	P6M1 = 0x00;P6M0 = 0x00;
	P7M1 = 0x00;P7M0 = 0x00;
	
	Timer0_Init(); //定时器0初始化
	GPS_UART_Init(); //GPS串口初始化
	// Init_GPS_Setting(); //GPS初始化
	Encoder_Init(); //编码器初始化
	SPI_Init(); //SPI初始化
	// SPI_InitSlave(); //SPI从模式初始化
	ICM42688_Init(); //陀螺仪初始化
	UART_Init(); //Debug串口初始化
	// LIS3MDL_Init();
}

void main()
{
	unsigned char GPS_Init = 1;
	Inits();
	Delay100us();
	
	
	while(1)
	{
		// UART_SendByte('S');
		// UART_SendStr("Hello World!\0");
		a = (float) rmc_data.valid;
		value[0] = a;
		UART_SendFloat(value);
		// #pragma region GPS数据模块
		// GPS_Message_Updater();
		// if (GPS_Init == 1)
		// {
		// 	GPS_Init = Init_GPS_Offset(&naturePosition, &rmc_data);
		// 	if (GPS_Init == 0)
		// 	{
		// 		UART_SendStr("GPS偏移量初始化成功!\0");
		// 		GPS_Init = 0;
		// 	}
		// }
		// #pragma endregion

		// #pragma region 陀螺仪数据模块
		// Gyro_Updater();
		// #pragma endregion

		// #pragma region 编码器数据模块
		// Encoder_Update();
		// #pragma endregion


		// #pragma region 磁场计数据模块
		// // LIS3MDL_ReadData(&mag_data); // 读取磁力计数据
		// #pragma endregion
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