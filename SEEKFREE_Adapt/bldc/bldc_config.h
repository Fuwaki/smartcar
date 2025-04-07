#ifndef _bldc_config_h_
#define _bldc_config_h_


// 35000000 / 24000 = 1458
// 35000000 为系统频率35Mhz
// 24000为24Khz

#define BLDC_MAX_DUTY                   (1458)     		// PWM的最大占空比，需要自行根据PWM初始化去计算
	
#define BLDC_MIN_DUTY                   (40)      		// PWM的最小占空比，根据输入油门大小计算出来的占空比小于此定义则电机停转 至少要大于40

#define BLDC_PWM_DEADTIME				(BLDC_MIN_DUTY)	// 死区

#define BLDC_MIN_BATTERY                (9000)   		// 最小电压，单位mv，当检测到电压低于此值电机停转
	
#define BLDC_COMMUTATION_FAILED_MAX     (200)     		// 换相错误最大次数 大于这个次数后认为电机堵转
	
#define BLDC_START_DELAY                (2000)   		// 当遇到堵转后电机停止时间，之后电机会再次尝试启动（单位0.05ms）

#define BLDC_CLOSE_LOOP_WAIT        	(50 )           // 闭环等待换向次数

#define BLDC_OPEN_LOOP_WAIT        		(50 )           // 闭环等待换向次数

#define BLDC_SPEED_INCREMENTAL      	(10)            	// 加减速响应 1-20，可修改这个提高加速响应，
														// 设置20响应速度最慢，设置1响应速度最快。
														// 响应速度越快，越容易出现换相错误，所以建议从20开始，一点一点减小。

#define BLDC_POLES                  	(7)             // 电机极对数

#define BLDC_BEEP_ENABLE                (0)      		// 1:使能上电电机鸣叫功能 0：禁用
	
#define BLDC_BEEP_VOLUME                (50 )    	 	// 电机鸣叫声音大小 0-100

#define BLDC_MOTOR_ANGLE				(25)			// 马达进角，进入比较器中断后，延时多少度进行换相
														// 范围0-30度

#define BLDC_USE_SINE_START             (0)   			// 使用正弦启动

#define BLDC_SINE_START_DUTY            (3)   		    // 正弦启动占空比 1-32 ,值越大正弦启动力越大。


#endif

