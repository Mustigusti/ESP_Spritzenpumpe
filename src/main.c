#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "flow_sensor.h"

void app_main()
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
        ESP_LOGE("main", "Sensor init failed");
        return;
    }

    if (flow_sensor_start_measurement() != 0)
    {
        ESP_LOGE("main", "Sensor measurement start failed");
        return;
    }

    vTaskDelay(60 / portTICK_PERIOD_MS); // Warm-up

    while (1)
    {
        vTaskDelay(12 / portTICK_PERIOD_MS);
        float flow = flow_sensor_read();
        ESP_LOGI("main", "The Flow Rate: %.2f µL/min", flow);
    }
}
