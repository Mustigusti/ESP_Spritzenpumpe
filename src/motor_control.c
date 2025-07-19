#include "motor_control.h"
#include "sensor_driver.h"
#include "config.h"
#include "driver/mcpwm.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <math.h>
#include <inttypes.h>

static const char* TAG = "MOTOR_CTRL";

static float set_point = 0;
static float integral = 0;
static float prev_error = 0;

static void pwm_init()
{
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, 18);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, 19);

    mcpwm_config_t pwm_config = {
        .frequency = PWM_FREQ_HZ,
        .cmpr_a = 0,
        .cmpr_b = 0,
        .counter_mode = MCPWM_UP_COUNTER,
        .duty_mode = MCPWM_DUTY_MODE_0
    };
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
}

void motor_control_init()
{
    pwm_init();
    sensor_driver_init();
}

void motor_control_set_point(float sp)
{
    set_point = sp;
}

void motor_control_run_pid()
{
    float flow_rate = sensor_driver_read_flow();
    float error = set_point - flow_rate;
    integral += error;
    float derivative = error - prev_error;

    float control_signal = K_P * error + K_I * integral + K_D * derivative;
    control_signal = fminf(fmaxf(control_signal, 0.0f), 1.0f);
    uint32_t duty = (uint32_t)(control_signal * MAX_DUTY);

    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, (double)duty * 100.0 / MAX_DUTY);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);

    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 0);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);

    prev_error = error;
    ESP_LOGI(TAG, "Setpoint=%.2f, Flow=%.2f, Duty=%u", set_point, flow_rate, (unsigned int)duty);
}

void motor_control_stop()
{
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);

    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 0);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);

}

void motor_control_reverse()
{
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);


    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 100);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);

}

