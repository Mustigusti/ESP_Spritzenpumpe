// main.c

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "motor_control.h"
#include "sensor_driver.h"
#include "config.h"
#include <string.h>
#include <stdlib.h>
#include "esp_rom_sys.h"
#include "uart_comm.h"

#define TAG "SUPER_LOOP"

// one global flag, start stopped
volatile float sp;
volatile bool is_running = false; // shared with uart_comm.c
volatile bool manual_mode = false;

void app_main(void)
{

    // --- 1) Init hardware ---

    // On‑board LED on GPIO2 (for debug blink, optional)
    gpio_config_t io = {
        .pin_bit_mask = (1ULL << GPIO_NUM_2),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io);

    // Power the flow sensor
    gpio_set_direction(GPIO_NUM_12, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_12, 1);

    // I2C sensor init
    sensor_driver_init();

    flow_sensor_start_measurement();

    // Motor control init + default setpoint
    motor_control_init();
    motor_control_set_point(1000.0f);

    // UART init (talk to Pico)
    uart_comm_init();

    static char rx_buffer[UART_BUF_SIZE];
    static size_t rx_index = 0;

    // --- 2) Super‐loop ---
    uint8_t byte;

    while (true)
    {
        while (uart_read_bytes(UART_PORT_NUM, &byte, 1, pdMS_TO_TICKS(10)) > 0)
        {
            if (byte == '\n')
            {
                rx_buffer[rx_index] = '\0';               // null-terminate the string
                ESP_LOGI(TAG, "Received: %s", rx_buffer); // 2a) Poll for a command (non‐blocking, 100 ms timeout)

                if (strncmp(rx_buffer, "START", 5) == 0)
                {
                    is_running = true;
                    manual_mode = false;
                }
                if (strncmp(rx_buffer, "STOP", 4) == 0)
                {
                    is_running = false;
                    manual_mode = false;
                    motor_control_stop();
                    gpio_set_level(GPIO_NUM_2, 0);
                }
                if (strncmp(rx_buffer, "REVERSE", 7) == 0)
                {
                    motor_control_reverse();
                    is_running = false;
                    manual_mode = true;
                    gpio_set_level(GPIO_NUM_2, 0);
                }
                if (strncmp(rx_buffer, "FORWARD", 7) == 0)
                {
                    motor_control_forward();
                    is_running = false;
                    manual_mode = true;
                    gpio_set_level(GPIO_NUM_2, 0);
                }
                if (strncmp(rx_buffer, "SETPOINT:", 9) == 0)
                {
                    sp = atof(&rx_buffer[9]);
                    motor_control_set_point(sp);
                    is_running = true;
                    manual_mode = false;
                }
                rx_index = 0; // reset index for next command
            }
            else
            {
                if (rx_index < sizeof(rx_buffer) - 1)
                {
                    rx_buffer[rx_index++] = byte; // store byte in buffer
                }
            }
        }

        // 2b) If running, do PID + send flow
        if (is_running)
        {
            // toggle LED so you can see loop running
            gpio_set_level(GPIO_NUM_2, 1);

            // scan_i2c_bus();
            float flow = motor_control_run_pid();

            char out[32];
            //scan_i2c_bus();
            int n = snprintf(out, sizeof(out), "FLOW:%.2f\n", flow);
            uart_write_bytes(UART_PORT_NUM, out, n);
        }
        else if (!manual_mode)
        {
            motor_control_stop();
            gpio_set_level(GPIO_NUM_2, 0);
        }
        vTaskDelay(pdMS_TO_TICKS(12));
        
    }
}
