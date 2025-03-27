#include <AI8051U.H>
#include <intrins.h>
#include "cameraController.h"


void Inits()
{


    
    CCD_Init();
}

void main()
{
    Inits();
    while(1)
    {
        StartCCD();
        // Do something with the binary data
    }
}