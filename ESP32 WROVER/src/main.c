// main.c

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "esp_log.h"
#include "motor_control.h"
#include "sensor_driver.h"
#include "uart_comm.h"
#include "config.h"
#include <string.h>
#include <stdlib.h>

#define TAG "MAIN"
#define TIMER_INTERVAL_MS 12

static QueueHandle_t pid_trigger_queue;

volatile float sp;
volatile bool is_running = false;
volatile bool manual_mode = false;

bool IRAM_ATTR timer_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    BaseType_t high_task_wakeup = pdFALSE;
    uint32_t dummy = 1;
    xQueueSendFromISR(pid_trigger_queue, &dummy, &high_task_wakeup);
    return high_task_wakeup == pdTRUE;
}

void pid_task(void *arg)
{
    uint32_t dummy;
    while (1)
    {
        if (xQueueReceive(pid_trigger_queue, &dummy, portMAX_DELAY))
        {
            if (is_running)
            {
                gpio_set_level(GPIO_NUM_2, 1);
                float flow = motor_control_run_pid();

                char out[32];
                int len = snprintf(out, sizeof(out), "FLOW:%.2f\n", flow);
                uart_write_bytes(UART_PORT_NUM, out, len);
            }
            else if (!manual_mode)
            {
                motor_control_stop();
                gpio_set_level(GPIO_NUM_2, 0);
            }
        }
    }
}

// If you didn't put flow_task in sensor_driver.c, define it here instead
/*
void flow_task(void *arg)
{
    while (1)
    {
        int flow = sensor_driver_read_flow();
        latest_flow = flow;
        vTaskDelay(pdMS_TO_TICKS(12));
    }
}
*/

void app_main(void)
{
    // Debug LED
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_NUM_2),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io_conf);

    // Power flow sensor
    gpio_set_direction(GPIO_NUM_12, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_12, 1);

    // Init sensor, motor, UART
    sensor_driver_init();
    flow_sensor_start_measurement();
    motor_control_init();
    motor_control_set_point(1000.0f);
    uart_comm_init();

    // Create PID queue and task
    pid_trigger_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(pid_task, "pid_task", 2048, NULL, 10, NULL);
    xTaskCreate(flow_task, "flow_task", 2048, NULL, 5, NULL);  // start flow reader

    // Set up and start GPTimer
    gptimer_handle_t timer = NULL;
    gptimer_config_t config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000,
    };
    gptimer_new_timer(&config, &timer);

    gptimer_event_callbacks_t callbacks = {
        .on_alarm = timer_callback,
    };
    gptimer_register_event_callbacks(timer, &callbacks, NULL);

    gptimer_alarm_config_t alarm_config = {
        .reload_count = 0,
        .alarm_count = TIMER_INTERVAL_MS * 1000,
        .flags.auto_reload_on_alarm = true,
    };
    gptimer_set_alarm_action(timer, &alarm_config);
    gptimer_enable(timer);
    gptimer_start(timer);

    // UART command handling
    static char rx_buffer[UART_BUF_SIZE];
    static size_t rx_index = 0;
    uint8_t byte;

    while (true)
    {
        while (uart_read_bytes(UART_PORT_NUM, &byte, 1, pdMS_TO_TICKS(10)) > 0)
        {
            if (byte == '\n')
            {
                rx_buffer[rx_index] = '\0';
                ESP_LOGI(TAG, "Received: %s", rx_buffer);

                if (strncmp(rx_buffer, "START", 5) == 0)
                {
                    is_running = true;
                    manual_mode = false;
                }
                else if (strncmp(rx_buffer, "STOP", 4) == 0)
                {
                    is_running = false;
                    manual_mode = false;
                    motor_control_stop();
                    gpio_set_level(GPIO_NUM_2, 0);
                }
                else if (strncmp(rx_buffer, "REVERSE", 7) == 0)
                {
                    motor_control_reverse();
                    is_running = false;
                    manual_mode = true;
                    gpio_set_level(GPIO_NUM_2, 0);
                }
                else if (strncmp(rx_buffer, "FORWARD", 7) == 0)
                {
                    motor_control_forward();
                    is_running = false;
                    manual_mode = true;
                    gpio_set_level(GPIO_NUM_2, 0);
                }
                else if (strncmp(rx_buffer, "SETPOINT:", 9) == 0)
                {
                    sp = atof(&rx_buffer[9]);
                    motor_control_set_point(sp);
                    is_running = true;
                    manual_mode = false;
                }

                rx_index = 0;
            }
            else if (rx_index < sizeof(rx_buffer) - 1)
            {
                rx_buffer[rx_index++] = byte;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
