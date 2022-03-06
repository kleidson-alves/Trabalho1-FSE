
#ifndef _SENSOR_
#define _SENSOR_

#include "bme280_defs.h"

int8_t user_i2c_read(uint8_t reg_addr, uint8_t* data, uint32_t len, void* intf_ptr);
void user_delay_us(uint32_t period, void* intf_ptr);
int8_t user_i2c_write(uint8_t reg_addr, const uint8_t* data, uint32_t len, void* intf_ptr);
void bme_init();
void bme_stop();
double get_temperatura();

#endif