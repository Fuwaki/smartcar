typedef struct {
    float kp;
    float ki;
    float kd;
    float last_error;
    float error_sum;
    float integral_limit;
    float output_limit;

}PID_Control;

void PID_Init(PID_Control *pc, float kp, float ki, float kd);
void PID_Set_Integral_Limit(PID_Control *pc, float limit);
void PID_Set_Output_Limit(PID_Control *pc, float limit);
float PID_Update(PID_Control* pc,float error,float dt);
