#ifndef SENSOR_DRIVER_H
#define SENSOR_DRIVER_H

#include "esp_err.h" // <— add this
#include "config.h"

void sensor_driver_init(void);
void flow_sensor_start_measurement(void);
float sensor_driver_read_flow(void);
void scan_i2c_bus(void);
void flow_task(void *arg);

extern volatile int latest_flow;

#endif // SENSOR_DRIVER_H
