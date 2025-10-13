#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "wireless_station_firmware.h"
#include "errors.h"


int main() {
    
    initialize_board();
    
    // Pointer to a ambient_info_t struct that stores all the data
    // read by the sensors.
    ambient_info_t sensor_readings;

    // Array of ambient_info_t elements that holds the n-previous
    // sensor readings. It's used to check for potential upcoming 
    // hazards, such as a flood, a sudden fire or a gas leak.
    ambient_info_t previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY];

    // Initialize the BH1750.
    uint8_t cmd = BH1750_CONT_H_RES_MODE;
    i2c_write_blocking(i2c0, LIGHT_SENSOR_I2C_ADDRESS, &cmd, 1, false);
    sleep_ms(180); // Wait for the first measurement (~120-180ms for high res).

    while (1) {
        sensor_readings.temperature = NAN;
        sensor_readings.humidity = NAN;
        sensor_readings.pressure = NAN;
        sensor_readings.air_quality_index = NAN;
        sensor_readings.light_intensity = NAN;

        if (read_temperature_and_humidity(&sensor_readings) == -1) {
            printf("Error when reading the temperature and humidity sensor. Error number %d\n",
                TEMP_HUMIDITY_READ_ERROR);
        }
        else {
            printf("Temperature: %.1fC, Humidity: %.1f%%\n",
                   sensor_readings.temperature, sensor_readings.humidity);
        }

        if (read_light_intensity(&sensor_readings) == -1) {
            printf("Error when reading the light intensity sensor. Error number %d\n",
                LIGHT_SENSOR_READ_ERROR);
        }
        else {
            printf("Light intensity: %.4f\n", sensor_readings.light_intensity);
        }

        for (int i = 0; i < LENGTH_PREVIOUS_READINGS_ARRAY - 1; i++) {
            previous_readings[i] = previous_readings[i + 1];
        }
        previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY - 1] = sensor_readings;

        int hazard_code = analyze_hazards(previous_readings);
        switch(hazard_code) {
            case TEMPERATURE_RISING_HAZARD:
                printf("ALERT: TEMPERATURE RISING!\n");
                break;
            case HUMIDITY_RISING_HAZARD:
                printf("ALERT: HUMIDITY RISING!\n");
                break;
            case PRESSURE_RISING_HAZARD:
                printf("ALERT: PRESSURE RISING!\n");
                break;
            case AIR_QUALITY_WORSENING_HAZARD:
                printf("ALERT: AIR QUALITY WORSENING!\n");
                break;
            default:
                break;
        }

    
        // Wait another full minute before reading sensors again.
        sleep_ms(1000);
    }
}