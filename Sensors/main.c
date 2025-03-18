#include <AI8051U.H>
#include <intrins.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "GPS.h"
#include "Encoder.h"
#include "ICM42688_SPI.h"
#include "SPI_MultiDevice.h"
#include "Timer.h"
#include "uart.h"
#include "Gyroscope.h"
//��д�������ĺ�������
void SPI_SEND(char *str){

}
void Construct_Data_Frame(struct Posture p){
	//Ҫ�ͺ��İ�����ݽ��ܵĽ���������ƥ��
	char *s = "";
	SPI_SEND(s);
}

// unsigned char receive_buffer[32]; // 可以根据需要调整大小
const char* s="qwq\0";
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
	EAXFR = 1; 									// 使能访问 XFR
	CKCON = 0x00; 							// 设置外部数据总线速度为最快
	WTST = 0x00;								// 设置程序代码等待参数，等待时间为0个时钟，CPU执行程序速度最快

	P0M1 = 0x00;P0M0 = 0x00;		// 设置P0口为准双向口模式 //00：准双向口 01：推挽输出 10：高阻输入 11：开漏输出
	P1M1 = 0x00;P1M0 = 0x00;		// 设置P1口为准双向口模式 //00：准双向口 01：推挽输出 10：高阻输入 11：开漏输出
	P2M1 = 0x00;P2M0 = 0x00;		// 设置P2口为准双向口模式 //00：准双向口 01：推挽输出 10：高阻输入 11：开漏输出
	P3M1 = 0x00;P3M0 = 0x00;		// 设置P3口为准双向口模式 //00：准双向口 01：推挽输出 10：高阻输入 11：开漏输出
	P4M1 = 0x00;P4M0 = 0x00;		// 设置P4口为准双向口模式 //00：准双向口 01：推挽输出 10：高阻输入 11：开漏输出
	P5M1 = 0x00;P5M0 = 0x00;		// 设置P5口为准双向口模式 //00：准双向口 01：推挽输出 10：高阻输入 11：开漏输出
	P6M1 = 0x00;P6M0 = 0x00;		// 设置P6口为准双向口模式 //00：准双向口 01：推挽输出 10：高阻输入 11：开漏输出
	P7M1 = 0x00;P7M0 = 0x00;		// 设置P7口为准双向口模式 //00：准双向口 01：推挽输出 10：高阻输入 11：开漏输出

	Encoder_Init();
	UART_Init();
	Timer0_Init();
	// Encoder_InterruptEnable(0x01);
	// ICM42688_SPI_Init();
}


void main()
{
	Inits();
<<<<<<< HEAD
	
=======
	Delay100us();
	UART_SendByte('S');
>>>>>>> refs/remotes/origin/main
	while(1)
	{
		// if(UART_Available())
		// {
		// 	unsigned char received_len;
		// 	received_len = UART_Read(receive_buffer, 32);
		// 	if(received_len > 0)
		// 	{
		// 		receive_bufferp[received_len] = '\0'
		// 		temp = UART_Read(unsigned char *buf, unsigned char len)
		// 		UART_SendByte(temp);
		// 	}
		// }
		Delay1000ms();
	}
}