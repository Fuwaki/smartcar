#ifndef COMM_N
#define COMN_N
// CANIDǰ����λ��ʶ���� ��8���ֽڱ�ʶ����
#define CANID 0x7
// 8λ��������
#define COMMAND_SPEED 0x1
#define COMMAND_LIVE 0x2
void OnCommandSpeed(float speed);
void Init_Listen();
void Listen_Update();
void Uart3_Init(void);
void Uart3Send(char dat);
void Uart3SendStr(char *p);
int MotorCommander();
void Uart3SendByLength(unsigned char *p, int length);
void OnCommandSpeed(float speed);
void OnCommandLive();
// void OnMsgRecv(unsigned int canid, unsigned char *dat, unsigned char len);
void Timer3_Init(void); // 1��@35.000MHz

typedef struct {
  unsigned char Motor_ID;
  float Motor_Speed;
} MOTOR_CONTROL_FRAME;

extern MOTOR_CONTROL_FRAME motor_control_frame; // �������֡�ṹ��

#endif