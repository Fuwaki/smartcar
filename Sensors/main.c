#include <AI8051U.H>
#include <intrins.h>
#include <string.h>
#include <stdlib.h>
#include "GPS.h"
#include "Gyroscope.h"


void Inits()
{
	GPS_Config(); // GPS初始化
}

void main()
{
	Inits();
	while(1)
	{
		
	}
}