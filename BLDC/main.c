#include <AI8051U.H>
#include <intrins.h>
#include "PWM_Controller.h"


void qwq()
{
	printf("fk u fuwaky\n");
	qwq();
}

void Delay100us(void)	//@12.000MHz
{
	unsigned long edata i;

	_nop_();
	_nop_();
	_nop_();
	i = 298UL;
	while (i) i--;
}

typedef (int)
void run(void * callback)
{
	
}

void main (void)
{
	int i = 0;
	P2M0 = 0X00;
	P2M1 = 0x00;
	while(1)
	{
		for (i = 0; i < 255;i++)
		{
			P2 = i;
			Delay10ms();

		}


	}
}
