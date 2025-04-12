#include "communication.h"
#include "motor_control.h"
#include "bldc_config.h"
#include "../../library/can.h"
static int alive=1;
void OnCommandLive(){
    //TODO:��Ӷ�ʱ�� ��ʱ��һ���ͳ���ɲ������ Ȼ����Ҫ������ʱ����������������ö�ʱ��
    alive=1;
    //��ʱ�����¿�ʼ����
    // T4T3M &= ~0x08;     // ֹͣ��ʱ��3
    // T3L = 0xD1;        //���ö�ʱ��ʼֵ
    // T3H = 0x02;        //���ö�ʱ��ʼֵ
    // T4T3M |= 0x08;     // ��ʱ��3��ʼ��ʱ
}

void OnCommandSpeed(float speed){
    if(speed>1.0f||speed<=0.0f||!alive){
        motor.duty=0;
    }else{
        motor.duty=(unsigned int)(BLDC_MIN_DUTY+speed*(BLDC_MAX_DUTY-BLDC_MIN_DUTY));
    }

}
void OnMsgRecv(unsigned int canid, unsigned char *dat,unsigned char len){
    unsigned char command=canid&0xff;       //ȡ��8���ֽھ�������
    float speed=0.0f;
    motor.duty=200;
    
    // if(command==COMMAND_SPEED){
    //     if(len!=4){
    //         //��ȡת��
    //         speed=*(float*)dat;
    //         OnCommandSpeed(speed); 
    //     }else{
    //         //TODO:��Ӵ�����
    //     }
    // } else if (command == COMMAND_LIVE) {
    //   OnCommandLive();
    // }else{
    //     //TODO:��Ӵ�����
    // }
    
}
void Timer3_Init(void)		//1��@35.000MHz
{
    TM3PS = 0x2C;			//���ö�ʱ��ʱ��Ԥ��Ƶ ( ע��:��������ϵ�ж��д˼Ĵ���,������鿴�����ֲ� )
    T4T3M &= 0xFD;			//��ʱ��ʱ��12Tģʽ
    T3L = 0xD1;				//���ö�ʱ��ʼֵ
    T3H = 0x02;				//���ö�ʱ��ʼֵ
    T4T3M |= 0x08;			//��ʱ��3��ʼ��ʱ
    ET3 = 1;  
}
//��ʱ��3���жϺ���
void TM3_Isr() interrupt TMR3_VECTOR{
    alive=0;
    if(motor.duty==100){
        motor.duty=0;
    }else{
        motor.duty=100;
    }
    P41=~P41;
}

void Init_Listen(){
    can_init();
    // can_set_filter(((unsigned int)CANID)<<8,0xff);      //ֻ����ǰ3λ��id
    can_set_receive_callback(OnMsgRecv);
    Timer3_Init();
}