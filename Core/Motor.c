#include "Motor.h"
#include "uart.h"
#define LEFT_BACK_MOTOR 0x45
#define RIGHT_BACK_MOTOR 0x11
#define LEFT_MOTOR 0x01
#define RIGHT_MOTOR 0x03
#define LEFT_BOTTOM_MOTOR 0x19
#define RIGHT_BOTTOM_MOTOR 0x14
static struct Motor_State previous_motor_state;
void Motor_Init()
{
    previous_motor_state.left = 0;
    previous_motor_state.right = 0;
    previous_motor_state.back_left = 0;
    previous_motor_state.back_right = 0;
    //在这里调整一下向下电机的速度 刚好让小船浮起来就好了
    previous_motor_state.bottom_left = 0.1;
    previous_motor_state.bottom_right = 0.1;
    // Motor_Apply_State(previous_motor_state);
}
void Motor_Apply_State(struct Motor_State state)
{
    MOTOR_CONTROL_FRAME motor_control_frame;
    // if(state.back_left!=previous_motor_state.back_left)
    {
        motor_control_frame.Motor_ID = LEFT_BACK_MOTOR;
        motor_control_frame.Motor_Speed = state.back_left;
        UART3_SendCommandToMotor(motor_control_frame);
    }
    // if(state.back_right!=previous_motor_state.back_right)
    {
        motor_control_frame.Motor_ID = RIGHT_BACK_MOTOR;
        motor_control_frame.Motor_Speed = state.back_right;
        UART3_SendCommandToMotor(motor_control_frame);
    }
    // if(state.left!=previous_motor_state.left)
    {
        motor_control_frame.Motor_ID = LEFT_MOTOR;
        motor_control_frame.Motor_Speed = state.left;
        UART3_SendCommandToMotor(motor_control_frame);
    }
    // if(state.right!=previous_motor_state.right)
    {
        motor_control_frame.Motor_ID = RIGHT_MOTOR;
        motor_control_frame.Motor_Speed = state.right;
        UART3_SendCommandToMotor(motor_control_frame);
    }
    // if(state.bottom_left!=previous_motor_state.bottom_left)
    {
        motor_control_frame.Motor_ID = LEFT_BOTTOM_MOTOR;
        motor_control_frame.Motor_Speed = state.bottom_left;
        UART3_SendCommandToMotor(motor_control_frame);
    }
    // if(state.bottom_right!=previous_motor_state.bottom_right)
    {
        motor_control_frame.Motor_ID = RIGHT_BOTTOM_MOTOR;
        motor_control_frame.Motor_Speed = state.bottom_right;
        UART3_SendCommandToMotor(motor_control_frame);
    }
    previous_motor_state = state;
}
