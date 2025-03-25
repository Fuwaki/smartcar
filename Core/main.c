#include <STC32G.H>
#include <intrins.h>
#include "AR_PF.h"
#include "Muti_SPI_Device.h"
#include "uart.h"
//QWQing
void Delay100ms(void)	//@35.000MHz
{
	unsigned long edata i;

	_nop_();
	_nop_();
	i = 874998UL;
	while (i) i--;
}


void main()
{
    EAXFR = 1;    // 使能访问 XFR,没有冲突不用关闭
    CKCON = 0x00; // 设置外部数据总线速度为最快
    WTST = 0x00;  // 设置程序代码等待参数，
    // 赋值为 0 可将 CPU 执行程序的速度设置为最快
    P0M0 = 0x00;
    P0M1 = 0x00;
    P1M0 = 0x00;
    P1M1 = 0x00;
    P2M0 = 0x00;
    P2M1 = 0x00;
    P3M0 = 0x00;
    P3M1 = 0x00;
    P4M0 = 0x00;
    P4M1 = 0x00;
    P5M0 = 0x00;
    P5M1 = 0x00;
    Uart3Init();
    ES3 = 1;
    EA = 1;

    while (1)
    {


        Uart3SendStr("Hello World!\0");
        Delay100ms();
    }
}