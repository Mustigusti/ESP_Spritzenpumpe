#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "string.h"
#include "motor_control.h"
#include "config.h"
#include <stdbool.h>
#include <stdlib.h>

static const char *TAG = "UART_COMM";
static TaskHandle_t uart_task_handle = NULL;

volatile bool is_running = true;

// Function to send flow data over UART
void uart_comm_send_flow(float flow_rate)
{
    char msg[32];
    snprintf(msg, sizeof(msg), "FLOW:%.2f\n", flow_rate);
    uart_write_bytes(UART_PORT_NUM, msg, strlen(msg));
    ESP_LOGI(TAG, "Sent: %s", msg);
}

// Function to receive command over UART
int uart_comm_receive_command(char *buffer, size_t max_len)
{
    int len = uart_read_bytes(UART_PORT_NUM, (uint8_t *)buffer, max_len - 1, pdMS_TO_TICKS(1000));
    if (len > 0)
    {
        buffer[len] = '\0';
    }
    return len;
}

static void uart_command_task(void *arg)
{
    uint8_t buf[UART_BUF_SIZE];
    while (1)
    {
        int len = uart_read_bytes(UART_PORT_NUM, buf, sizeof(buf) - 1, pdMS_TO_TICKS(1000));
        if (len > 0)
        {
            buf[len] = '\0';
            ESP_LOGI(TAG, "Received: '%s'", buf);

            if (strncmp((char *)buf, "START", 5) == 0)
            {
                motor_control_init();
                is_running = true;
                ESP_LOGI(TAG, "Command: START");
            }
            else if (strncmp((char *)buf, "STOP", 4) == 0)
            {
                motor_control_stop();
                is_running = false;
                ESP_LOGI(TAG, "Command: STOP");
            }
            else if (strncmp((char *)buf, "REVERSE", 7) == 0)
            {
                motor_control_reverse();
                ESP_LOGI(TAG, "Command: REVERSE");
            }
            else if (strncmp((char *)buf, "FORWARD", 7) == 0)
            {
                motor_control_forward();
                ESP_LOGI(TAG, "Command: FORWARD");
            }
            else if (strncmp((char *)buf, "SETPOINT:", 9) == 0)
            {
                float sp = atof((char *)&buf[9]);
                motor_control_set_point(sp);
                ESP_LOGI(TAG, "Command: SETPOINT = %.2f", sp);
            }
            else
            {
                ESP_LOGW(TAG, "Unknown command");
            }
        }
    }
}

void uart_comm_init(void)
{
    uart_config_t uart_conf = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_conf));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, UART_BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_LOGI(TAG, "UART initialized on TX=%d RX=%d", UART_TX_PIN, UART_RX_PIN);

    xTaskCreatePinnedToCore(uart_command_task, "uart_cmd_task", 4096, NULL, 5, &uart_task_handle, 1);
}
