// flow_sensor.c
#include "flow_sensor.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "flow_sensor_config.h"

static const char *TAG = "flow_sensor";

static uint8_t write_data[] = {
    SENSOR_CMD_START_CONTINUOUS_MEASUREMENT_0,
    SENSOR_CMD_START_CONTINUOUS_MEASUREMENT_1};

static uint8_t read_data[6];

static bool check_crc(uint8_t *data)
{
    uint8_t calculated_crc = CRC_INIT_VALUE;
    for (int i = 0; i < 2; i++)
    {
        calculated_crc ^= data[i];
        for (int j = 0; j < 8; j++)
        {
            if (calculated_crc & 0x80)
            {
                calculated_crc = (calculated_crc << 1) ^ POLYNOMIAL;
            }
            else
            {
                calculated_crc <<= 1;
            }
        }
    }
    uint8_t received_crc = data[2];
    return calculated_crc == received_crc;
}

esp_err_t flow_sensor_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ};

    esp_err_t ret = i2c_param_config(I2C_NUM_0, &conf);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C config failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C driver install failed: %s", esp_err_to_name(ret));
    }
    return ret;
}

int flow_sensor_start_measurement(void)
{
    esp_err_t err = i2c_master_write_to_device(
        I2C_NUM_0,
        SENSOR_ADDR,
        write_data,
        sizeof(write_data),
        1000 / portTICK_PERIOD_MS);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C Write failed: %s", esp_err_to_name(err));
        return -1;
    }
    return 0;
}

float flow_sensor_read(void)
{
    esp_err_t err = i2c_master_read_from_device(
        I2C_NUM_0,
        SENSOR_ADDR,
        read_data,
        sizeof(read_data),
        1000 / portTICK_PERIOD_MS);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C Read failed: %s", esp_err_to_name(err));
        return -1;
    }

    if (!check_crc(read_data))
    {
        ESP_LOGE(TAG, "CRC check failed");
        return -1;
    }

    int16_t flow_raw = (read_data[0] << 8) | read_data[1];
    float flow_rate = (float)flow_raw / 10.0f;

    if (flow_rate > 3250)
    {
        flow_rate = 0;
    }

    return flow_rate;
}
