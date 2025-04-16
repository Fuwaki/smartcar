#include "communication.h"
#include "motor_control.h"
#include "bldc_config.h"

bit busy114;
char wptr;
char rptr;
char buffer[16];

unsigned char receive_buffer[16]; // ���ջ�����
unsigned char receive_state = 0;  // ����״̬

MOTOR_CONTROL_FRAME motor_control_frame; // ��ʱ���������ݽṹ��
static unsigned long alive = 1;

void OnCommandLive()
{
    // TODO:��Ӷ�ʱ�� ��ʱ��һ���ͳ���ɲ������ Ȼ����Ҫ������ʱ����������������ö�ʱ��
    alive = 1;
    // ��ʱ�����¿�ʼ����
    //  T4T3M &= ~0x08;     // ֹͣ��ʱ��3
    //  T3L = 0xD1;        //���ö�ʱ��ʼֵ
    //  T3H = 0x02;        //���ö�ʱ��ʼֵ
    //  T4T3M |= 0x08;     // ��ʱ��3��ʼ��ʱ
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
//     unsigned char command = canid & 0xff; // ȡ��8���ֽھ�������
//     float speed = 0.0f;
//     motor.duty = 200;

//     // if(command==COMMAND_SPEED){
//     //     if(len!=4){
//     //         //��ȡת��
//     //         speed=*(float*)dat;
//     //         OnCommandSpeed(speed);
//     //     }else{
//     //         //TODO:��Ӵ�����
//     //     }
//     // } else if (command == COMMAND_LIVE) {
//     //   OnCommandLive();
//     // }else{
//     //     //TODO:��Ӵ�����
//     // }
// }

//void Timer3_Init(void) // 100ms@35.000MHz
//{
//    T4T3M |= 0x02; // ��ʱ��ʱ��1Tģʽ
//   T3L = 0x54;    // ���ö�ʱ��ʼֵ
//    T3H = 0xF2;    // ���ö�ʱ��ʼֵ
//    T4T3M |= 0x08; // ��ʱ��3��ʼ��ʱ
//    ET3 = 1;
//}
void Timer3_Init(void)		//100����@35.000MHz
{
	TM3PS = 0x04;			//���ö�ʱ��ʱ��Ԥ��Ƶ ( ע��:��������ϵ�ж��д˼Ĵ���,������鿴�����ֲ� )
	T4T3M &= 0xFD;			//��ʱ��ʱ��12Tģʽ
	T3L = 0x23;				//���ö�ʱ��ʼֵ
	T3H = 0x1C;				//���ö�ʱ��ʼֵ
	T4T3M |= 0x08;			//��ʱ��3��ʼ��ʱ
}
// ��ʱ��3���жϺ���
void TM3_Isr() interrupt TMR3_VECTOR
{
	T3L = 0x23;				//���ö�ʱ��ʼֵ
	T3H = 0x1C;				//���ö�ʱ��ʼֵ
    // alive ++;
    MotorCommander();
    motor.duty=60;
    // P41 = ~P41;
    // if(alive>=10){
    //     motor.duty = 0; // 3��û���յ������ֹͣ���
    // }

}



void Init_Listen()
{
    //can_init();
    // can_set_filter(((unsigned int)CANID)<<8,0xff);      //ֻ����ǰ3λ��id
    // can_set_receive_callback(OnMsgRecv);
    // Timer3_Init();
    Uart3_Init(); // ��ʼ������3
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
    S3CON = 0x10;  // 8λ����,�ɱ䲨����
    S3CON &= 0xBF; // ����3ѡ��ʱ��2Ϊ�����ʷ�����
    AUXR |= 0x04;  // ��ʱ��ʱ��1Tģʽ
    T2L = 0xB4;    // ���ö�ʱ��ʼֵ
    T2H = 0xFF;    // ���ö�ʱ��ʼֵ
    AUXR |= 0x10;  // ��ʱ��2��ʼ��ʱ
    IE2 |= 0x08;   // ʹ�ܴ���3�ж�

    PS3 = 1;
    PS3H = 1; // ���ô���3�ж����ȼ�Ϊ��

    ES3 = 1; // ʹ�ܴ���3�ж�
    EA = 1;  // ʹ�����ж�
    wptr=0;
    rptr=0;
}
void Listen_Update(){
    int has_new_data=MotorCommander();
    //FIXME:�����ʱ��ʽ̫��ª�� ���ת�ٲ�ͬ��ʱ��alive��ʱ��Ҳ��ͬ
    if(alive>300000){
        motor.duty = 0; // ����ʱ���û���յ������ֹͣ���
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
    unsigned char check_frame[4] = {0x00, 0x00, 0x80, 0x7f}; // ֡ͷ
    unsigned char *p = (unsigned char *)&motor_control_frame;
    unsigned int i;

    while (rptr != wptr)
    {
        data_byte = buffer[rptr++];
        rptr &= 0x0f; // ѭ��������

        if (receive_state < 4) // ��ʼ״̬
        {
            if (data_byte == check_frame[receive_state])
            {
                receive_state++;
                if (receive_state == 4) // ֡ͷ�������
                {
                    // ���ý��ջ�����������׼����������
                    receive_state = 4;
                }
            }
            else
            {
                // ֡ͷ��ƥ�䣬����״̬
                receive_state = 0;
                // �����ǰ�ֽڿ�����֡ͷ�ĵ�һ���ֽڣ���Ҫ���¼��
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

            if (receive_state == 9) // ֡ͷ�������
            {
                receive_state = 0;        // ����״̬��׼��������һ֡
                p[0] = receive_buffer[0]; // ���ID
                p[1] = receive_buffer[1];
                p[2] = receive_buffer[2];
                p[3] = receive_buffer[3];
                p[4] = receive_buffer[4];
                flag=1;
            }
        }
        else
        {
            receive_state = 0; // ����״̬��׼��������һ֡
            if (data_byte == check_frame[0])
            {
                receive_state = 1; // �����ǰ�ֽڿ�����֡ͷ�ĵ�һ���ֽڣ���Ҫ���¼��
            }
        }
    }
    return flag;
}