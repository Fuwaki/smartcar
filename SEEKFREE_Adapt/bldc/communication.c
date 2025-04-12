#include "communication.h"
#include "motor_control.h"
#include "bldc_config.h"
#include "../../library/can.h"
static int alive=1;
void OnCommandLive(){
    //TODO:添加定时器 定时器一满就出发刹车保护 然后需要主机定时发送这个命令来重置定时器
    alive=1;
    //定时器重新开始计数
    // T4T3M &= ~0x08;     // 停止定时器3
    // T3L = 0xD1;        //设置定时初始值
    // T3H = 0x02;        //设置定时初始值
    // T4T3M |= 0x08;     // 定时器3开始计时
}

void OnCommandSpeed(float speed){
    if(speed>1.0f||speed<=0.0f||!alive){
        motor.duty=0;
    }else{
        motor.duty=(unsigned int)(BLDC_MIN_DUTY+speed*(BLDC_MAX_DUTY-BLDC_MIN_DUTY));
    }

}
void OnMsgRecv(unsigned int canid, unsigned char *dat,unsigned char len){
    unsigned char command=canid&0xff;       //取后8个字节就是命令
    float speed=0.0f;
    motor.duty=200;
    
    // if(command==COMMAND_SPEED){
    //     if(len!=4){
    //         //读取转速
    //         speed=*(float*)dat;
    //         OnCommandSpeed(speed); 
    //     }else{
    //         //TODO:添加错误处理
    //     }
    // } else if (command == COMMAND_LIVE) {
    //   OnCommandLive();
    // }else{
    //     //TODO:添加错误处理
    // }
    
}
void Timer3_Init(void)		//1秒@35.000MHz
{
    TM3PS = 0x2C;			//设置定时器时钟预分频 ( 注意:并非所有系列都有此寄存器,详情请查看数据手册 )
    T4T3M &= 0xFD;			//定时器时钟12T模式
    T3L = 0xD1;				//设置定时初始值
    T3H = 0x02;				//设置定时初始值
    T4T3M |= 0x08;			//定时器3开始计时
    ET3 = 1;  
}
//定时器3的中断函数
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
    // can_set_filter(((unsigned int)CANID)<<8,0xff);      //只关心前3位的id
    can_set_receive_callback(OnMsgRecv);
    Timer3_Init();
}