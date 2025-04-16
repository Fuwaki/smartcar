#include "communication.h"
#include "motor_control.h"
#include "bldc_config.h"

bit busy114;
char wptr;
char rptr;
char buffer[16];

unsigned char receive_buffer[16]; // 接收缓冲区
unsigned char receive_state = 0;  // 接收状态

MOTOR_CONTROL_FRAME motor_control_frame; // 临时传感器数据结构体
static unsigned long alive = 1;

void OnCommandLive()
{
    // TODO:添加定时器 定时器一满就出发刹车保护 然后需要主机定时发送这个命令来重置定时器
    alive = 1;
    // 定时器重新开始计数
    //  T4T3M &= ~0x08;     // 停止定时器3
    //  T3L = 0xD1;        //设置定时初始值
    //  T3H = 0x02;        //设置定时初始值
    //  T4T3M |= 0x08;     // 定时器3开始计时
}

void OnCommandSpeed(float speed)
{
    if (speed > 1.0f || speed <= 0.0f )
    {
        motor.duty = 0;
    }
    else
    {
        motor.duty = (unsigned int)(BLDC_MIN_DUTY + speed * (BLDC_MAX_DUTY - BLDC_MIN_DUTY));
    }
}

// void OnMsgRecv(unsigned int canid, unsigned char *dat, unsigned char len)
// {
//     unsigned char command = canid & 0xff; // 取后8个字节就是命令
//     float speed = 0.0f;
//     motor.duty = 200;

//     // if(command==COMMAND_SPEED){
//     //     if(len!=4){
//     //         //读取转速
//     //         speed=*(float*)dat;
//     //         OnCommandSpeed(speed);
//     //     }else{
//     //         //TODO:添加错误处理
//     //     }
//     // } else if (command == COMMAND_LIVE) {
//     //   OnCommandLive();
//     // }else{
//     //     //TODO:添加错误处理
//     // }
// }

//void Timer3_Init(void) // 100ms@35.000MHz
//{
//    T4T3M |= 0x02; // 定时器时钟1T模式
//   T3L = 0x54;    // 设置定时初始值
//    T3H = 0xF2;    // 设置定时初始值
//    T4T3M |= 0x08; // 定时器3开始计时
//    ET3 = 1;
//}
void Timer3_Init(void)		//100毫秒@35.000MHz
{
	TM3PS = 0x04;			//设置定时器时钟预分频 ( 注意:并非所有系列都有此寄存器,详情请查看数据手册 )
	T4T3M &= 0xFD;			//定时器时钟12T模式
	T3L = 0x23;				//设置定时初始值
	T3H = 0x1C;				//设置定时初始值
	T4T3M |= 0x08;			//定时器3开始计时
}
// 定时器3的中断函数
void TM3_Isr() interrupt TMR3_VECTOR
{
	T3L = 0x23;				//设置定时初始值
	T3H = 0x1C;				//设置定时初始值
    // alive ++;
    MotorCommander();
    motor.duty=60;
    // P41 = ~P41;
    // if(alive>=10){
    //     motor.duty = 0; // 3秒没有收到命令就停止电机
    // }

}



void Init_Listen()
{
    //can_init();
    // can_set_filter(((unsigned int)CANID)<<8,0xff);      //只关心前3位的id
    // can_set_receive_callback(OnMsgRecv);
    // Timer3_Init();
    Uart3_Init(); // 初始化串口3
	//motor.duty=200;
}

void Uart3_Isr(void) interrupt 17
{
    if (S3TI)
    {
        S3TI = 0;
        busy114 = 0;
    }
    if (S3RI)
    {
        S3RI = 0;
        buffer[wptr++] = S3BUF;
        wptr &= 0x0f;
    }
}

void Uart3_Init(void) // 115200bps@35.000MHz
{
    S3CON = 0x10;  // 8位数据,可变波特率
    S3CON &= 0xBF; // 串口3选择定时器2为波特率发生器
    AUXR |= 0x04;  // 定时器时钟1T模式
    T2L = 0xB4;    // 设置定时初始值
    T2H = 0xFF;    // 设置定时初始值
    AUXR |= 0x10;  // 定时器2开始计时
    IE2 |= 0x08;   // 使能串口3中断

    PS3 = 1;
    PS3H = 1; // 设置串口3中断优先级为高

    ES3 = 1; // 使能串口3中断
    EA = 1;  // 使能总中断
    wptr=0;
    rptr=0;
}
void Listen_Update(){
    int has_new_data=MotorCommander();
    //FIXME:这个定时方式太简陋了 电机转速不同的时候alive的时间也不同
    if(alive>300000){
        motor.duty = 0; // 若干时间后没有收到命令就停止电机
    }else{
        alive++;
    }
    if(has_new_data){
        if(motor_control_frame.Motor_ID!=MOTOR_ID){
            return;
        }
        alive=0;
        OnCommandSpeed(motor_control_frame.Motor_Speed);
    }
    P41=~P41;
    
}
void Uart3Send(char dat)
{
    while (busy114)
        ;
    busy114 = 1;
    S3BUF = dat;
}

void Uart3SendStr(char *p)
{
    while (*p)
    {
        Uart3Send(*(p++));
    }
}

void Uart3SendByLength(unsigned char *p, int length)
{
    int i;
    p += length - 1;
    for (i = 0; i < length; i++)
    {
        Uart3Send(*(p--));
    }
}

int MotorCommander()
{
    int flag=0;
    unsigned char data_byte;
    unsigned char check_frame[4] = {0x00, 0x00, 0x80, 0x7f}; // 帧头
    unsigned char *p = (unsigned char *)&motor_control_frame;
    unsigned int i;

    while (rptr != wptr)
    {
        data_byte = buffer[rptr++];
        rptr &= 0x0f; // 循环缓冲区

        if (receive_state < 4) // 初始状态
        {
            if (data_byte == check_frame[receive_state])
            {
                receive_state++;
                if (receive_state == 4) // 帧头接收完成
                {
                    // 重置接收缓冲区索引，准备接收数据
                    receive_state = 4;
                }
            }
            else
            {
                // 帧头不匹配，重置状态
                receive_state = 0;
                // 如果当前字节可能是帧头的第一个字节，需要重新检查
                if (data_byte == check_frame[0])
                {
                    receive_state = 1;
                }
            }
        }
        else if (receive_state >= 4 && receive_state < 9)
        {
            receive_buffer[receive_state - 4] = data_byte;
            receive_state++;

            if (receive_state == 9) // 帧头接收完成
            {
                receive_state = 0;        // 重置状态，准备接收下一帧
                p[0] = receive_buffer[0]; // 电机ID
                p[1] = receive_buffer[1];
                p[2] = receive_buffer[2];
                p[3] = receive_buffer[3];
                p[4] = receive_buffer[4];
                flag=1;
            }
        }
        else
        {
            receive_state = 0; // 重置状态，准备接收下一帧
            if (data_byte == check_frame[0])
            {
                receive_state = 1; // 如果当前字节可能是帧头的第一个字节，需要重新检查
            }
        }
    }
    return flag;
}