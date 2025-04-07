#include "communication.h"
#include "motor_control.h"

void OnMsgRecv(){

}
void OnCommandLive(){
    //TODO:添加定时器 定时器一满就出发刹车保护 然后需要主机定时发送这个命令来重置定时器
}

void OnCommandSpeed(float speed){
    if (speed>100.0||speed<0.0) return;
    motor.duty=(unsigned short)speed;
}
void Init_Listen(){
    can_init();
    can_set_filter(((unsigned int)CANID)<<8,0xff);      //只关心前3位的id
    
}