#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

void motor_control_init();
void motor_control_set_point(float set_point);
void motor_control_run_pid();
void motor_control_stop();

#endif // MOTOR_CONTROL_H
