#include "freertos/FreeRTOS.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "string.h"
#include "stdlib.h"
#include "uart_comm.h"
#include "config.h"

static const char *TAG = "UART_COMM";

// shared running flag from main.c
extern volatile bool is_running;

void uart_comm_init(void)
{
    uart_config_t uart_conf = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_conf));
    ESP_ERROR_CHECK(uart_set_pin(
        UART_PORT_NUM,
        UART_TX_PIN,
        UART_RX_PIN,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE));
    // install driver: RX buffer only, TX write directly
    ESP_ERROR_CHECK(uart_driver_install(
        UART_PORT_NUM,
        UART_BUF_SIZE * 2,
        0,
        0,
        NULL,
        0));

    // ESP_LOGI(TAG, "UART initialized (port %d, TX=%d, RX=%d)",
            //  UART_PORT_NUM, UART_TX_PIN, UART_RX_PIN);
}

int uart_comm_receive_command(char *buffer, size_t max_len)
{
    // read up to max_len-1 bytes, wait up to 100ms
    int len = uart_read_bytes(
        UART_PORT_NUM,
        (uint8_t *)buffer,
        max_len - 1,
        pdMS_TO_TICKS(100));
    if (len > 0)
    {
        buffer[len] = '\0';
    }
    return len;
}

void uart_comm_send_flow(float flow_rate)
{
    char msg[32];
    int n = snprintf(msg, sizeof(msg), "FLOW:%.2f\n", flow_rate);
    if (n > 0)
    {
        uart_write_bytes(UART_PORT_NUM, msg, n);
    }
    // avoid ESP_LOG here to prevent recursive UART use
}
