#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
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

        // 12-bit conversion, assume max value == ADC_VREF == 3.3 V
        const float conversion_factor = 3.3f / (1 << 12);
        uint16_t result = adc_read();
        printf("Raw value: 0x%03x, voltage: %f V\n", result, result * conversion_factor);
        sleep_ms(200);
    }
}

void read_temperature_and_humidity(dht22_reading *result) {
    int data[5] = {0, 0, 0, 0, 0};
    uint j = 0;
    uint last = 1;

    // Send start signal
    gpio_set_dir(DHT22_PIN, GPIO_OUT);
    gpio_put(DHT22_PIN, 0);
    sleep_ms(20);
    gpio_set_dir(DHT22_PIN, GPIO_IN);

    for (uint i = 0; i < MAX_TIMINGS; i++) {
        // measure pulse length in µs
        uint32_t start_time = time_us_32();
        while (gpio_get(DHT22_PIN) == last) {
            if ((time_us_32() - start_time) > 1000) break; // 1 ms timeout
        }
        uint32_t pulse_length = time_us_32() - start_time;
        last = gpio_get(DHT22_PIN);

        if (pulse_length > 1000) break; // sensor stopped responding

        // starting from bit 0 after ~4 transitions
        if ((i >= 4) && (i % 2 == 0)) {
            data[j / 8] <<= 1;
            if (pulse_length > 35) data[j / 8] |= 1;  // >35 µs = 1, else 0
            j++;
        }
    }

    // Verify checksum
    if ((j >= 40) &&
        (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF))) {

        result->humidity = (float) ((data[0] << 8) + data[1]) / 10;
        if (result->humidity > 100) result->humidity = data[0]; // DHT11 fallback

        result->temp_celsius = (float) (((data[2] & 0x7F) << 8) + data[3]) / 10;
        if (result->temp_celsius > 125) result->temp_celsius = data[2];

        if (data[2] & 0x80) result->temp_celsius = -result->temp_celsius; // negative temp
    }

}



