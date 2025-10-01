#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "wireless_station.h"
#include "errors.h"


int main() {
    stdio_init_all();
    i2c_init(i2c0, I2C_BAUDRATE);
    gpio_init(DHT22_PIN);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    // Before your while loop, initialize the BH1750
    uint8_t cmd = BH1750_CONT_H_RES_MODE;
    i2c_write_blocking(i2c0, LIGHT_SENSOR_I2C_ADDRESS, &cmd, 1, false);
    sleep_ms(180); // wait for first measurement (~120-180ms for high res)

    // Now inside your loop, read 2 bytes
    uint8_t buf[2];
    int bytes_read = i2c_read_blocking(i2c0, LIGHT_SENSOR_I2C_ADDRESS, buf, 2, false);

    // Variables passed to the reading functions. They will store the values
    // read by the different sensors.
    dht22_reading dht22_reading;
    float light_intensity_reading;

    while (1) {
        dht22_reading.humidity = NAN;
        dht22_reading.temp_celsius = NAN;

        if (read_temperature_and_humidity(&dht22_reading) == -1) {
            printf("Error when reading the temperature and humidity sensor. Error number %d\n",
                TEMP_HUMIDITY_READ_ERROR);
        }
        else {
            printf("Humidity: %.1f%%, Temperature: %.1fC\n",
                   dht22_reading.humidity, dht22_reading.temp_celsius);
        }

        if (read_light_intensity(&light_intensity_reading) == -1) {
            printf("Error when reading the light intensity sensor. Error number %d\n",
                LIGHT_SENSOR_READ_ERROR);
        }
        else {
            printf("Light intensity: %.4f\n", light_intensity_reading);
        }

        sleep_ms(1000);
    }
}
