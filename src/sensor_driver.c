#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "sensor_driver.h"
#include "config.h"
#include <string.h>

#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_TIMEOUT_MS 1000

#define LFS6000_ADDR 0x08
#define LFS6000_COMMAND_MEASURE 0x3608

#define FLOW_TASK_STACK_SIZE 2048
#define FLOW_TASK_PRIORITY 9

static const char *TAG = "FLOW_SENSOR";
static TaskHandle_t flow_task_handle = NULL;

// ----------- Sensor I2C INIT -----------

void sensor_driver_init(void)
{
    static bool i2c_initialized = false;
    if (i2c_initialized)
    {
        ESP_LOGW(TAG, "I2C already initialized, skipping...");
        return;
    }

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    esp_err_t err;

    err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "i2c_param_config failed: %s", esp_err_to_name(err));
        return;
    }

    err = i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "i2c_driver_install failed: %s", esp_err_to_name(err));
        return;
    }

    i2c_initialized = true;
    ESP_LOGI(TAG, "I2C initialized");
}

// ----------- Start Flow Sensor -----------
void flow_sensor_start_measurement(void)
{
    uint8_t cmd[2] = {(LFS6000_COMMAND_MEASURE >> 8) & 0xFF, LFS6000_COMMAND_MEASURE & 0xFF};
    esp_err_t ret = i2c_master_write_to_device(
        I2C_MASTER_NUM, LFS6000_ADDR, cmd, sizeof(cmd), pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    if (ret == ESP_OK)
        ESP_LOGI(TAG, "Flow sensor measurement started");
    else
        ESP_LOGE(TAG, "Measurement start failed: %s", esp_err_to_name(ret));
}

// ----------- Read Flow Value -----------
float sensor_driver_read_flow(void)
{
    uint8_t data[3] = {0};
    esp_err_t ret = i2c_master_read_from_device(
        I2C_MASTER_NUM, LFS6000_ADDR, data, sizeof(data), pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));

    if (ret != ESP_OK)
    {
        //ESP_LOGE(TAG, "Flow read failed: %s", esp_err_to_name(ret));
        return 0.0f;
    }

    float raw_flow = (data[0] << 8) | data[1];

    float flow = (raw_flow) / 10; // Adjust scale as needed
    if (flow > 3260)
    {
        flow = 0.0f;
    }

    return flow;
}

// ----------- Flow Reading Task -----------
static void flow_task(void *arg)
{
    while (true)
    {
        float flow = sensor_driver_read_flow();
        ESP_LOGI(TAG, "Flow = %.2f slm", flow);
    }
}