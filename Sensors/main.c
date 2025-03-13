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