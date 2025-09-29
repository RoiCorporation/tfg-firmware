#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "wireless_station.h"


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


float read_co_mq7() {
    // uint16_t range = MQ_7_UPPER_LIMIT - MQ_7_LOWER_LIMIT;
    // return (BOARD_ADC_RESOLUTION / range) * adc_read() + MQ_7_LOWER_LIMIT;

    const float conv_factor = 3.3f / (1 << 12);
    float divider_gain = 1.5;
    float r0 = 70;
    float rl = 10000;

    uint16_t raw = adc_read();
    float v_out = raw * conv_factor;          // Voltage at ADC pin
    float v_sensor = v_out * divider_gain;    // Undo divider → sensor voltage

    // Calculate Rs
    float rs = rl * (5.0 - v_sensor) / v_sensor;

    // Normalize against baseline R0
    float ratio = rs / r0;

    //printf("Raw: %u, Vout: %.3f V, Vsensor: %.3f V, Rs: %.1f Ω, Rs/R0: %.3f\n",
    //        raw, v_out, v_sensor, rs, ratio);

    float rs_ro = rs / r0;

    // coefficients from datasheet fit
    const float m = -1.53f;    // slope
    const float b = 2.30f;     // intercept

    float log_ppm = m * log10f(rs_ro) + b;
    return powf(10, log_ppm);
}