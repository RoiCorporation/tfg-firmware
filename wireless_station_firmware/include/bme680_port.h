#ifndef BME680_PORT_H
#define BME680_PORT_H


#include "bme68x.h"


int8_t bme68x_platform_init(struct bme68x_dev *dev);
BME68X_INTF_RET_TYPE user_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);
BME68X_INTF_RET_TYPE user_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);
void user_delay_us(uint32_t period, void *intf_ptr);


#endif
