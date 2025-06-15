#ifndef SENSOR_DRIVER_H
#define SENSOR_DRIVER_H

void sensor_driver_init(void);
void flow_sensor_start_measurement(void);
float sensor_driver_read_flow(void);
void flow_task_create(void);  // ✅ Needed to spawn reading task

#endif // SENSOR_DRIVER_H
