#include "Track.h"
#define BASE_SPEED 0.5
#define LOWEST_SPEED 0.2
void Track_Init(){

}
struct BoatState Track_Update(){

}
struct BoatState Track_Update(){

}
struct Motor_State Track_Control(struct BoatState state){
    struct Motor_State motor_state;
    if(state.An>0){
        //左转
        motor_state.right=BASE_SPEED+state.An;
        motor_state.left=BASE_SPEED;
    }else{
        //右转
        motor_state.left=BASE_SPEED-state.An;
        motor_state.right=BASE_SPEED;
    }
    motor_state.back_left=state.At-state.M;
    motor_state.back_right=state.At+state.M;
    //不可以小于最低速度以防止电机停转
    motor_state.back_left=motor_state.back_left>LOWEST_SPEED?motor_state.back_left:LOWEST_SPEED;
    motor_state.back_right=motor_state.back_right>LOWEST_SPEED?motor_state.back_right:LOWEST_SPEED;
    return motor_state;
}
