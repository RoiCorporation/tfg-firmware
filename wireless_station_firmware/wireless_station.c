#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "wireless_station.h"


int main() {
    stdio_init_all();
    adc_init();
    i2c_init(i2c0, I2C_BAUDRATE);
    gpio_init(DHT22_PIN);
    adc_gpio_init(MQ_7_PIN);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    adc_select_input(0);

    // Before your while loop, initialize the BH1750
    uint8_t cmd = BH1750_CONT_H_RES_MODE;
    i2c_write_blocking(i2c0, LIGHT_SENSOR_I2C_ADDRESS, &cmd, 1, false);
    sleep_ms(180); // wait for first measurement (~120-180ms for high res)

    // Now inside your loop, read 2 bytes
    uint8_t buf[2];
    int bytes_read = i2c_read_blocking(i2c0, LIGHT_SENSOR_I2C_ADDRESS, buf, 2, false);


    while (1) {
        dht22_reading dht22_reading;
        dht22_reading.humidity = NAN;
        dht22_reading.temp_celsius = NAN;

        read_temperature_and_humidity(&dht22_reading);

        if (!isnan(dht22_reading.humidity) && !isnan(dht22_reading.temp_celsius)) {
            float fahrenheit = (dht22_reading.temp_celsius * 9 / 5) + 32;
            printf("Humidity = %.1f%%, Temperature = %.1fC (%.1fF)\n",
                   dht22_reading.humidity, dht22_reading.temp_celsius, fahrenheit);
        }
        sleep_ms(2000); // wait before next sample

        uint8_t cmd = BH1750_CONT_H_RES_MODE;
        i2c_write_blocking(i2c0, LIGHT_SENSOR_I2C_ADDRESS, &cmd, 1, false);

        // Wait for conversion (max 180 ms)
        sleep_ms(180);

        // Read 2 bytes
        uint8_t buf[2];
        int bytes_read = i2c_read_blocking(i2c0, LIGHT_SENSOR_I2C_ADDRESS, buf, 2, false);

        if (bytes_read == 2) {
            uint16_t raw = (buf[0] << 8) | buf[1];
            float lux = raw / 1.2f;
            printf("Light intensity: %.2f lux\n", lux);
        } else {
            printf("I2C read failed!\n");
        }

        printf("PPM of CO detected: %.2f\n", read_co_mq7());
        sleep_ms(200);
    }
}
