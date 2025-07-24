#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gptimer.h"
#include "motor_control.h"
#include "sensor_driver.h"
#include "uart_comm.h"
#include "config.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

static const char *TAG = "MAIN";

// PID task handle
static TaskHandle_t pid_task_handle = NULL;

// Timer handle
static gptimer_handle_t timer = NULL;

// PID task
static void pid_task(void *arg)
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if (is_running) // from uart_comm.c: volatile bool is_running = true;

        {
            float flow = motor_control_run_pid(); //returns flow rate of the syringe
            uart_comm_send_flow(flow); //sends flow to the other microcontroller(pico) via UART
        }
        else
        {
            motor_control_stop();
        }
    }
}

// UART command task
static void uart_command_task(void *arg)
{
    char buffer[UART_BUF_SIZE];

    while (true)
    {
        int len = uart_comm_receive_command(buffer, sizeof(buffer));
        if (len > 0)
        {
            ESP_LOGI(TAG, "UART CMD: '%s'", buffer);

            if (strncmp(buffer, "START", 5) == 0)
            {
                motor_control_init();
                is_running = true;
                ESP_LOGI(TAG, "Command: START");
            }
            else if (strncmp(buffer, "STOP", 4) == 0)
            {
                motor_control_stop();
                is_running = false;
                ESP_LOGI(TAG, "Command: STOP");
            }
            else if (strncmp(buffer, "REVERSE", 7) == 0)
            {
                motor_control_reverse();
                ESP_LOGI(TAG, "Command: REVERSE");
            }
            else if (strncmp(buffer, "FORWARD", 7) == 0)
            {
                motor_control_forward();
                ESP_LOGI(TAG, "Command: FORWARD");
            }
            else if (strncmp(buffer, "SETPOINT:", 9) == 0)
            {
                float sp = atof(&buffer[9]);
                motor_control_set_point(sp);
                ESP_LOGI(TAG, "Command: SETPOINT = %.2f", sp);
            }
            else
            {
                ESP_LOGW(TAG, "Unknown UART command");
            }
        }
    }
}

// ISR callback
static bool IRAM_ATTR timer_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(pid_task_handle, &xHigherPriorityTaskWoken);
    return xHigherPriorityTaskWoken == pdTRUE;
}

void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_INFO);
    ESP_LOGI(TAG, "🚨 System Booting - UART controlled mode");

    // Initialize UART communication
    uart_comm_init();

    // Start UART command task
    xTaskCreatePinnedToCore(uart_command_task, "uart_cmd_task", 4096, NULL, 5, NULL, 1);

    // GPIO12 powers the sensor
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_NUM_12),
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level(GPIO_NUM_12, 1);

    // Setup sensor
    sensor_driver_init();
    flow_sensor_start_measurement();

    // Init motor
    motor_control_init();
    motor_control_set_point(1000.0f);

    // Start PID task
    xTaskCreatePinnedToCore(pid_task, "pid_task", 4096, NULL, 10, &pid_task_handle, 1);

    // Configure GPTimer
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000,
    };
    gptimer_new_timer(&timer_config, &timer);

    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_callback,
    };
    gptimer_register_event_callbacks(timer, &cbs, NULL);

    gptimer_set_raw_count(timer, 0);

    gptimer_alarm_config_t alarm_config = {
        .alarm_count = PID_INTERVAL_MS * 1000,
        .flags.auto_reload_on_alarm = true,
    };
    gptimer_set_alarm_action(timer, &alarm_config);

    gptimer_enable(timer);
    gptimer_start(timer);

    ESP_LOGI(TAG, "System initialized and running");
}
