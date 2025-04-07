#ifndef _bldc_config_h_
#define _bldc_config_h_


// 35000000 / 24000 = 1458
// 35000000 ΪϵͳƵ��35Mhz
// 24000Ϊ24Khz

#define BLDC_MAX_DUTY                   (1458)     		// PWM�����ռ�ձȣ���Ҫ���и���PWM��ʼ��ȥ����
	
#define BLDC_MIN_DUTY                   (40)      		// PWM����Сռ�ձȣ������������Ŵ�С���������ռ�ձ�С�ڴ˶�������ͣת ����Ҫ����40

#define BLDC_PWM_DEADTIME				(BLDC_MIN_DUTY)	// ����

#define BLDC_MIN_BATTERY                (9000)   		// ��С��ѹ����λmv������⵽��ѹ���ڴ�ֵ���ͣת
	
#define BLDC_COMMUTATION_FAILED_MAX     (200)     		// ������������� ���������������Ϊ�����ת
	
#define BLDC_START_DELAY                (2000)   		// ��������ת����ֹͣʱ�䣬֮�������ٴγ�����������λ0.05ms��

#define BLDC_CLOSE_LOOP_WAIT        	(50 )           // �ջ��ȴ��������

#define BLDC_OPEN_LOOP_WAIT        		(50 )           // �ջ��ȴ��������

#define BLDC_SPEED_INCREMENTAL      	(10)            	// �Ӽ�����Ӧ 1-20�����޸������߼�����Ӧ��
														// ����20��Ӧ�ٶ�����������1��Ӧ�ٶ���졣
														// ��Ӧ�ٶ�Խ�죬Խ���׳��ֻ���������Խ����20��ʼ��һ��һ���С��

#define BLDC_POLES                  	(7)             // ���������

#define BLDC_BEEP_ENABLE                (0)      		// 1:ʹ���ϵ������й��� 0������
	
#define BLDC_BEEP_VOLUME                (50 )    	 	// �������������С 0-100

#define BLDC_MOTOR_ANGLE				(25)			// �����ǣ�����Ƚ����жϺ���ʱ���ٶȽ��л���
														// ��Χ0-30��

#define BLDC_USE_SINE_START             (0)   			// ʹ����������

#define BLDC_SINE_START_DUTY            (3)   		    // ��������ռ�ձ� 1-32 ,ֵԽ������������Խ��


#endif

