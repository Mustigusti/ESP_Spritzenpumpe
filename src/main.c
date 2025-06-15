#include "motor_control.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config.h"
#include "driver/gptimer.h"
#include "sensor_driver.h"
#include "driver/gpio.h"



// Timer handle (global, not re-declared in app_main)
static gptimer_handle_t timer = NULL;

// PID task handle for notification
static TaskHandle_t pid_task_handle = NULL;

// PID task: runs motor_control_run_pid() safely in task context
static void pid_task(void *arg)
{
    while (true)
    {
        // Wait for notification from ISR
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        motor_control_run_pid();
    }
}

// ISR callback (do NOT call motor_control_run_pid here!)
static bool IRAM_ATTR timer_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(pid_task_handle, &xHigherPriorityTaskWoken);
    return xHigherPriorityTaskWoken == pdTRUE; // yield if higher priority task was woken
}

void app_main(void)
{
    // Power on sensor via GPIO12
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_NUM_12),
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level(GPIO_NUM_12, 1); // Set HIGH to power sensor
    vTaskDelay(pdMS_TO_TICKS(500));  // Wait for sensor to boot
    // Sensor setup
    sensor_driver_init();
    vTaskDelay(pdMS_TO_TICKS(100));

    flow_sensor_start_measurement();
    vTaskDelay(pdMS_TO_TICKS(100));

    //flow_task_create();
    //vTaskDelay(pdMS_TO_TICKS(100));

    // Init motor and setpoint
    motor_control_init();
    motor_control_set_point(120.0);
    vTaskDelay(pdMS_TO_TICKS(100));


    // Start PID task
    xTaskCreatePinnedToCore(pid_task, "pid_task", 4096, NULL, 10, &pid_task_handle, 1); // Run on core 1

    // Configure GPTimer
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1 MHz = 1 tick = 1 microsecond
    };
    gptimer_new_timer(&timer_config, &timer);  // use global timer, don't redeclare

    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_callback,
    };
    gptimer_register_event_callbacks(timer, &cbs, NULL);

    gptimer_set_raw_count(timer, 0);

    gptimer_alarm_config_t alarm_config = {
        .alarm_count = PID_INTERVAL_MS * 1000, // ms to us
        .flags.auto_reload_on_alarm = true,
    };
    gptimer_set_alarm_action(timer, &alarm_config);

    gptimer_enable(timer);
    gptimer_start(timer);
}
