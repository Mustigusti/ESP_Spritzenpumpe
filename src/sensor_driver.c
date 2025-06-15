#include "sensor_driver.h"
#include "config.h"
#include "driver/i2c.h"
#include "esp_log.h"

static const char* TAG = "SENSOR";

void sensor_driver_init()
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

float sensor_driver_read_flow()
{
    uint8_t cmd[] = {0x36, 0x08};
    uint8_t data[3];

    if (i2c_master_write_read_device(I2C_MASTER_NUM, SENSOR_ADDR, cmd, 2, data, 3, pdMS_TO_TICKS(50)) != ESP_OK) {
        ESP_LOGW(TAG, "I2C read failed");
        return 0;
    }

    uint16_t value = ((uint16_t)data[0] << 8) | data[1];
    float flow = value / 10.0f;
    return (flow > 3250) ? 0 : flow;
}
