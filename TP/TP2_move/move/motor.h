#ifndef MOTOR_H
#define MOTOR_H


void motor_init(void);
void motor_set_speed(float speed_r, float speed_l);
void motor_set_position(float position_r, float position_l, float speed_r, float speed_l);
void motor_stop(void);

void motor_right_stop(void);
void motor_left_stop(void);
void wait_motor_done(void);
#endif /* MOTOR_H */
