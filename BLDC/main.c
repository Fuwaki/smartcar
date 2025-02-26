#include <AI8051U.H>
void qwq(){
	qwq();
}
void Delay10ms(void)	//@11.0592MHzr
{
	unsigned long edata i;
	i = 27646UL;
	while (i) i--;
}
typedef (int)
void run(void * callback){
	
}

void main (void)
{
	int i = 0;
	P2M0 = 0X00;
	P2M1 = 0x00;
	while(1)
	{
		for (i = 0; i < 255;i++){
			P2 = i;
			Delay10ms();

		}


	}
}
