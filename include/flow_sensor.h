#ifndef FLOW_SENSOR_H
#define FLOW_SENSOR_H

#include "esp_err.h"

esp_err_t flow_sensor_init(void);
int flow_sensor_start_measurement(void);
float flow_sensor_read(void);

#endif // FLOW_SENSOR_H
