#include "communication.h"
#include "motor_control.h"
#include "../../library/can.h"
void OnMsgRecv(){

}
void OnCommandLive(){
    //TODO:��Ӷ�ʱ�� ��ʱ��һ���ͳ���ɲ������ Ȼ����Ҫ������ʱ����������������ö�ʱ��
}

void OnCommandSpeed(float speed){
//32     motor.duty=(unsigned short)speed;
}
void Init_Listen(){
    can_init();
            
    P41=~P41;
    // can_set_filter(((unsigned int)CANID)<<8,0xff);      //ֻ����ǰ3λ��id
    
}