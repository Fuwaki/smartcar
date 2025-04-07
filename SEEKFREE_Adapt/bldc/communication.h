#ifndef COMM_N
#define COMN_N
//CANID前三个位标识对象 后8个字节标识命令
#define CANID 0x7
//8位的命令字
#define COMMAND_SPEED 0x1
#define COMMAND_LIVE 0x2
void Init_Listen();


#endif