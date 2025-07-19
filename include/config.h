#ifndef CONFIG_H
#define CONFIG_H

#define MAX_DUTY            65535
#define I2C_MASTER_SCL_IO   22
#define I2C_MASTER_SDA_IO   21
#define I2C_MASTER_NUM      I2C_NUM_0
#define I2C_MASTER_FREQ_HZ  400000
#define SENSOR_ADDR         0x08
#define PWM_FREQ_HZ         100000
#define PID_INTERVAL_MS     12

// PID constants (can be adjusted later)
#define K_U 0.004227f
#define T_U 0.176571f
#define K_P (0.6f * K_U * 0.4f)
#define K_I (2.0f * K_P / T_U * 0.001f)
#define K_D (K_P * T_U / 8.0f * 1.1f)

#endif // CONFIG_H