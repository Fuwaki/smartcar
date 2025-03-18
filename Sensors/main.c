#include <AI8051U.H>
#include <intrins.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "GPS.h"
#include "Encoder.h"
#include "ICM42688_SPI.h"
#include "SPI_MultiDevice.h"
#include "Gyroscope.h"
//先写个这样的函数丢这
void SPI_SEND(char *str){

}
void Construct_Data_Frame(struct Posture p){
	//要和核心板的数据接受的解析代码相匹配
	char *s = "";
	SPI_SEND(s);
}
void Inits()
{
	Encoder_Init();
	// Encoder_InterruptEnable(0x01);
	// ICM42688_SPI_Init();
}


void main()
{
	Inits();
	
	while(1)
	{

	}
}