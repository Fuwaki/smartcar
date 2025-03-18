#ifndef __GPS_H__
#define __GPS_H__
/*
��ʼ��ʱ�Ե�ǰ�㽨������ϵ ��ǰ����Ϊy��������
*/

//��̬��Ϣ
struct Posture{
    float angular_velocity[3];      //���ٶ� �ֱ��Ǹ����� ����� ƫ����  ǰ���߶�����ˮƽΪ0�� ƫ����������˵������һ��������0�㶼�� Ȼ��ֻ������ ˳ʱ����� תһȦ����
    float velocity[3];              //���ٶ�
    float attitude[3];              //��̬ ������� Ҳ�ֱ��Ǹ����� ����� ƫ���� ��ͽ��ٶ�һ��
    float position[2];              //������Խ�����ϵ
};

struct Posture get_fusion_data();
#endif