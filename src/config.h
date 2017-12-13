#ifndef __CONFIG_H
#define __CONFIG_H

#include <stdint.h>

//Nathan
//Struct with the id of each sensors
typedef struct _sensors_t {
  uint8_t color_sensor;
  uint8_t gyro_sensor;
  uint8_t ultrasonic_sensor;
  uint8_t compass_sensor;
  uint8_t touch_sensor;
} sensors_t;

sensors_t config();

#endif