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
volatile bool is_running = false; // shared with uart_comm.c

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
    uart_config_t ucfg = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    
    uart_comm_init();
    char cmd_buf[UART_BUF_SIZE];
    bool led = true;

    // --- 2) Super‐loop ---
    while (1)
    {

        ESP_LOGI(TAG, "Running main loop, is_running=%d", is_running);

        // 2a) Poll for a command (non‐blocking, 100 ms timeout)
        int len = uart_read_bytes(UART_PORT_NUM,
                                  (uint8_t *)cmd_buf,
                                  sizeof(cmd_buf) - 1,
                                  pdMS_TO_TICKS(100));
        if (len > 0)
        {
            cmd_buf[len] = '\0';
            if (strncmp(cmd_buf, "START", 5) == 0)
                is_running = true;
            else if (strncmp(cmd_buf, "STOP", 4) == 0)
                is_running = false;
            else if (strncmp(cmd_buf, "REVERSE", 7) == 0)
                motor_control_reverse();
            else if (strncmp(cmd_buf, "FORWARD", 7) == 0)
                motor_control_forward();
            else if (strncmp(cmd_buf, "SETPOINT:", 9) == 0)
            {
                float sp = atof(&cmd_buf[9]);
                motor_control_set_point(sp);
                is_running = true;
            }
            // (ignore unrecognized)
        }

        // 2b) If running, do PID + send flow
        if (is_running)
        {
            // toggle LED so you can see loop running
            gpio_set_level(GPIO_NUM_2, led);
            // led = !led;

            float flow = motor_control_run_pid();
            // send back "FLOW:xx.xx\n"
            char out[32];
            int n = snprintf(out, sizeof(out), "FLOW:%.2f\n", flow);
            uart_write_bytes(UART_PORT_NUM, out, n);
        }
        else
        {
            motor_control_stop();
        }
        vTaskDelay(pdMS_TO_TICKS(12));
        // esp_rom_delay_us(12000);

        // 2c) Delay ~12 ms before next loop
        // while (true)
        // {
        //     gpio_set_level(GPIO_NUM_2, 1);
        //     vTaskDelay(pdMS_TO_TICKS(500));
        //     gpio_set_level(GPIO_NUM_2, 0);
        // }
    }
}
