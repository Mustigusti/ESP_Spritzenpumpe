#include "motor_control.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config.h"
#include "driver/gptimer.h"
#include "esp_log.h"

static const char* TAG = "MAIN";

static gptimer_handle_t timer = NULL;

static bool IRAM_ATTR timer_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    motor_control_run_pid();  // call directly from ISR
    return true; // return true to yield to a higher-priority task if needed
}

void app_main(void)
{
    motor_control_init();
    motor_control_set_point(120.0);

    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1 MHz -> 1 tick = 1 us
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &timer));

    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_callback,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(timer, &cbs, NULL));

    ESP_ERROR_CHECK(gptimer_set_raw_count(timer, 0));

    gptimer_alarm_config_t alarm_config = {
        .alarm_count = PID_INTERVAL_MS * 1000,  // convert ms to us
        .flags.auto_reload_on_alarm = true,
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(timer, &alarm_config));
    ESP_ERROR_CHECK(gptimer_enable(timer));
    ESP_ERROR_CHECK(gptimer_start(timer));

    ESP_LOGI(TAG, "GPTimer started to run PID every %d ms", PID_INTERVAL_MS);
}
