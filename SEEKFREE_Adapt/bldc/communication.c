#include "communication.h"
#include "motor_control.h"

void OnMsgRecv(){

}
void OnCommandLive(){
    //TODO:��Ӷ�ʱ�� ��ʱ��һ���ͳ���ɲ������ Ȼ����Ҫ������ʱ����������������ö�ʱ��
}

void OnCommandSpeed(float speed){
    if (speed>100.0||speed<0.0) return;
    motor.duty=(unsigned short)speed;
}
void Init_Listen(){
    can_init();
    can_set_filter(((unsigned int)CANID)<<8,0xff);      //ֻ����ǰ3λ��id
    
}