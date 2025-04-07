typedef struct {
    float kp;
    float ki;
    float kd;

}PID_Control;

void PID_Init(PID_Control *pc, float kp, float ki, float kd);
float PID_Update(PID_Control* pc,float error);