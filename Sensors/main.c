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
//��д�������ĺ�������
void SPI_SEND(char *str){

}
void Construct_Data_Frame(struct Posture p){
	//Ҫ�ͺ��İ�����ݽ��ܵĽ���������ƥ��
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