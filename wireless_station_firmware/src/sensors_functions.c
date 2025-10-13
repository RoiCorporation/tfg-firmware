#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "wireless_station_firmware.h"
#include "errors.h"


/**
 * @brief initializes the different board components, such as stdio, I2C and GPIO.
 */
void initialize_board() {
    stdio_init_all();
    i2c_init(i2c0, I2C_BAUDRATE);
    gpio_init(DHT22_PIN);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);
}


/**
 * @brief reads temperature and humidity from the DHT22 sensor.
 * 
 * @param reading pointer to an ambient_info_t struct where the read values will be stored.
 * @return int 0 if the reading was successful, else -1.
 */
int read_temperature_and_humidity(ambient_info_t *reading) {
    int data[5] = {0, 0, 0, 0, 0};
    uint j = 0;
    uint last = 1;

    // Send start signal.
    gpio_set_dir(DHT22_PIN, GPIO_OUT);
    gpio_put(DHT22_PIN, 0);
    sleep_ms(20);
    gpio_set_dir(DHT22_PIN, GPIO_IN);

    for (uint i = 0; i < MAX_TIMINGS; i++) {
        // Measure pulse length in µs,
        uint32_t start_time = time_us_32();
        while (gpio_get(DHT22_PIN) == last) {
            if ((time_us_32() - start_time) > 1000) break; // 1 ms timeout,
        }
        uint32_t pulse_length = time_us_32() - start_time;
        last = gpio_get(DHT22_PIN);

        if (pulse_length > 1000) break; // Sensor stopped responding.

        // Starting from bit 0 after ~4 transitions.
        if ((i >= 4) && (i % 2 == 0)) {
            data[j / 8] <<= 1;
            if (pulse_length > 35) data[j / 8] |= 1;  // >35 µs = 1, else 0.
            j++;
        }
    }

    // Verify checksum. If successful, return 0, else return -1.
    if ((j >= 40) &&
        (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF))) {

        reading->humidity = (float) ((data[0] << 8) + data[1]) / 10;
        if (reading->humidity > 100) reading->humidity = data[0]; // DHT11 fallback.

        reading->temperature = (float) (((data[2] & 0x7F) << 8) + data[3]) / 10;
        if (reading->temperature > 125) reading->temperature = data[2];

        if (data[2] & 0x80) reading->temperature = -reading->temperature; // Negative temp.

        return 0;
    }

    reading->temperature = NAN;
    reading->humidity = NAN;
    return -1;
}


/**
 * @brief reads light intensity from the light sensor.
 * 
 * @param reading pointer to an ambient_info_t struct where the read values will be stored.
 * @return int 0 if the reading was successful, else -1.
 */
int read_light_intensity(ambient_info_t *reading) {
    // Tries to read 2 bytes.
    uint8_t buf[2];
    int bytes_read = i2c_read_blocking(i2c0, LIGHT_SENSOR_I2C_ADDRESS, buf, 2, false);

    // If successful, calculates the actual light intensity in lux. Else, return -1.
    if (bytes_read == 2) {
        uint16_t raw = (buf[0] << 8) | buf[1];
        float lux = raw / 1.2f;
        reading->light_intensity = lux;
        return 0;
    } 
    
    return -1;
}


/**
 * @brief checks if there are any potential hazards in the vicinity of the
 * station by analyzing the past few measurements taken by all sensors.
 * 
 * @param previous_readings the list of past measurements taken by the sensors.
 * @return int 0 if no hazards were detected, else the numeric code that
 * represents that particular hazard.
 */
int analyze_hazards(ambient_info_t previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY]) {
    bool is_temp_rising = true, 
        is_humidity_rising = true, 
        is_pressure_rising = true, 
        is_air_quality_worsening = true;

    // Loop through the array and check if any of the ambient conditions is worsening.
    for (int i = 1; i < LENGTH_PREVIOUS_READINGS_ARRAY; i++) {
        if (!is_temp_rising || 
            (previous_readings[i].temperature <= previous_readings[i - 1].temperature) || 
            ((previous_readings[i].temperature - previous_readings[i - 1].temperature) < TEMPERATURE_INCREASE_BUFFER))
            is_temp_rising = false;
        if (!is_humidity_rising || 
            (previous_readings[i].humidity <= previous_readings[i - 1].humidity) ||
            ((previous_readings[i].humidity - previous_readings[i - 1].humidity) < HUMIDITY_INCREASE_BUFFER))
            is_humidity_rising = false;
        if (!is_pressure_rising || 
            (previous_readings[i].pressure <= previous_readings[i - 1].pressure) ||
            ((previous_readings[i].pressure - previous_readings[i - 1].pressure) < PRESSURE_INCREASE_BUFFER))
            is_pressure_rising = false;
        if (!is_air_quality_worsening || 
            (previous_readings[i].air_quality_index >= previous_readings[i - 1].air_quality_index) ||
            ((previous_readings[i].air_quality_index - previous_readings[i - 1].air_quality_index) > AIR_QUALITY_WORSENING_BUFFER))
            is_air_quality_worsening = false;
    }

    // If any of the conditions are worsening, return that hazard code. Else, return 0.
    if (is_temp_rising) return TEMPERATURE_RISING_HAZARD;
    if (is_humidity_rising) return HUMIDITY_RISING_HAZARD;
    //if (is_pressure_rising) return PRESSURE_RISING_HAZARD;
    //if (is_air_quality_worsening) return AIR_QUALITY_WORSENING_HAZARD;
    return 0;
}