#include "Error.h"
int error_flag=0;
char *error_msg=0;
void ERROR(int flag, char *msg)
{
    error_flag=flag;
    error_msg=msg;
    // TODO:���ͳһ�Ĵ�����
}
