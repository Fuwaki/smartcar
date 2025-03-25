#ifndef __FUSION_H__
#define __FUSION__

/*
��ʼ��ʱ�Ե�ǰ�㽨������ϵ ��ǰ����Ϊy��������
*/
void Mahony_Init();
void Mahony_Update(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);
void Mahony_Get_Gravity_Vector(float *x,float *y,float *z);           //��ȡ�����ķ��� ʹ�ü��ٶȵ�ʱ����Ҫ��ȥ����
void Mahony_Get_Quaternion(float *w,float *x,float *y,float *z);
void Mahony_Get_Angular_Velocity(float *roll, float *pitch, float *yaw);

//��̬��Ϣ
struct Posture{
    float angular_velocity[3];      //���ٶ� �ֱ��Ǹ����� ����� ƫ����  ǰ���߶�����ˮƽΪ0�� ƫ����������˵������һ��������0�㶼�� Ȼ��ֻ������ ˳ʱ����� תһȦ����
    float velocity[3];              //���ٶ�
    float attitude[3];              //��̬ ������� Ҳ�ֱ��Ǹ����� ����� ƫ���� ��ͽ��ٶ�һ��
    float position[2];              //������Խ�����ϵ
};

struct Posture get_fusion_data();
#endif