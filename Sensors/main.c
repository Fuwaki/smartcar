#include <AI8051U.H>
#include <intrins.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "GPS.h"
#include "Encoder.h"
#include "SPI_MultiDevice.h"
#include "Gyroscope.h"
#include "Timer.h"
#include "uart.h"
#include "GPS_uart.h"
//先写个这样的函数丢这
// void SPI_SEND(char *str){

// }
// void Construct_Data_Frame(struct Posture p){
// 	//要和核心板的数据接受的解析代码相匹配
// 	char *s = "";
// 	SPI_SEND(s);
// }

unsigned char receive_buffer0d00[32];
unsigned char receive_buffer0d01[32];
char temp = 0;

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
	

	Encoder_Init();
	Timer0_Init();
	UART_Init();
	GPS_UART_Init();
	Init_GPS(&naturePosition, &rmc_data);
	SPI_Init();
	ICM42688_Init();
}

void main()
{
	Inits();
	Delay100us();
	UART_SendByte('S');
	while(1)
	{
		Gyro_Updater();
		GPS_Message_Updater();
		if(UART_Available())
		{
			unsigned char received_len;
			received_len = UART_Read(receive_buffer0d00, 32);
			if(received_len > 0)
			{
				receive_buffer0d00[received_len] = '\0';  // 添加字符串结束符
				// 通过串口发送接收到的数据
				UART_SendStr(receive_buffer0d00);
			}
		}
	}
}