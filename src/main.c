#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "esp_log.h"
#include "flow_sensor.h"

static const char *TAG = "main";
static TaskHandle_t flow_task_handle = NULL;

static bool IRAM_ATTR timer_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    BaseType_t high_task_wakeup = pdFALSE;
    vTaskNotifyGiveFromISR(flow_task_handle, &high_task_wakeup);
    return high_task_wakeup == pdTRUE;
}

void flow_task(void *arg)
{
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // Wait for timer interrupt
        float flow = flow_sensor_read();
        ESP_LOGI(TAG, "Flow Rate: %.2f µL/min", flow);
    }
}

void app_main(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << 12,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&io_conf);

    gpio_set_level(12, 1);

    if (flow_sensor_init() != ESP_OK)
    {
        ESP_LOGE(TAG, "Sensor init failed");
        return;
    }

    if (flow_sensor_start_measurement() != 0)
    {
        ESP_LOGE(TAG, "Sensor measurement start failed");
        return;
    }

    vTaskDelay(60 / portTICK_PERIOD_MS); // Warm-up

    xTaskCreate(flow_task, "flow_task", 2048, NULL, 5, &flow_task_handle);

    gptimer_handle_t gptimer = NULL;
    gptimer_config_t config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1 MHz = 1 us resolution
    };
    gptimer_new_timer(&config, &gptimer);

    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_callback,
    };
    gptimer_register_event_callbacks(gptimer, &cbs, NULL);

    gptimer_alarm_config_t alarm_config = {
        .alarm_count = 12000, // 12 ms = 12,000 us
        .reload_count = 0,
        .flags.auto_reload_on_alarm = true,
    };
    gptimer_set_alarm_action(gptimer, &alarm_config);
    gptimer_enable(gptimer);
    gptimer_start(gptimer);
}
