#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "bme680_port.h"


#define BME68X_I2C_PORT i2c0
#define BME68X_SDA_PIN  4
#define BME68X_SCL_PIN  5
#define BME68X_ADDR     BME68X_I2C_ADDR_LOW   // 0x76


static uint8_t bme_addr = BME68X_ADDR;

BME68X_INTF_RET_TYPE user_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    uint8_t addr = *(uint8_t *)intf_ptr;

    int w = i2c_write_blocking(BME68X_I2C_PORT, addr, &reg_addr, 1, true);
    if (w < 0) return BME68X_E_COM_FAIL;

    int r = i2c_read_blocking(BME68X_I2C_PORT, addr, reg_data, len, false);
    if (r < 0) return BME68X_E_COM_FAIL;

    return BME68X_INTF_RET_SUCCESS;
}

BME68X_INTF_RET_TYPE user_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    uint8_t addr = *(uint8_t *)intf_ptr;

    uint8_t buffer[32];
    if (len + 1 > sizeof(buffer)) return BME68X_E_COM_FAIL;

    buffer[0] = reg_addr;
    for (uint32_t i = 0; i < len; i++) {
        buffer[i + 1] = reg_data[i];
    }

    int w = i2c_write_blocking(BME68X_I2C_PORT, addr, buffer, len + 1, false);
    if (w < 0) return BME68X_E_COM_FAIL;

    return BME68X_INTF_RET_SUCCESS;
}

void user_delay_us(uint32_t period, void *intf_ptr)
{
    (void)intf_ptr;
    sleep_us(period);
}

int8_t bme68x_platform_init(struct bme68x_dev *dev)
{
    i2c_init(BME68X_I2C_PORT, 100 * 1000); // 100 kHz
    gpio_set_function(BME68X_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(BME68X_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(BME68X_SDA_PIN);
    gpio_pull_up(BME68X_SCL_PIN);

    dev->intf = BME68X_I2C_INTF;
    dev->read = user_i2c_read;
    dev->write = user_i2c_write;
    dev->delay_us = user_delay_us;
    dev->intf_ptr = &bme_addr;
    dev->amb_temp = 25;

    return BME68X_OK;
}
