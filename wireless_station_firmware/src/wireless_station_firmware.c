#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "bme68x.h"
#include "nrf24_driver.h"
#include "aes.h"
#include "wireless_station_firmware.h"
#include "hazards.h"
#include "alerts.h"
#include "errors.h"


int main() {

    // Pointer to a ambient_info_t struct that stores all the data
    // read by the sensors.
    ambient_info_t sensor_readings;
    
    // Array of ambient_info_t elements that holds the n-previous
    // sensor readings. It's used to check for potential upcoming 
    // hazards, such as a flood, a sudden fire or a gas leak.
    ambient_info_t previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY];

    // BME680 sensor driver and its configuration and heater structures.
    struct bme68x_dev bme680_sensor;
    struct bme68x_conf bme680_conf;
    struct bme68x_heatr_conf bme680_heater_conf;
    
    // NRF24L01 module driver.
    nrf_client_t nrf24_module;

    // AES encryption engine context.
    struct AES_ctx aes_ctx;

    uint8_t radio_message[sizeof(ambient_info_t)];

    // Configure all the protocols, devices and pins in the station.
    initialize_board(
        &bme680_sensor,
        &bme680_conf,
        &bme680_heater_conf,
        &nrf24_module,
        COPI_PIN,
        CIPO_PIN,
        SCK_PIN,
        CS_PIN,
        CE_PIN,
        SPI_BAUDRATE
    );

    while (!stdio_usb_connected()) sleep_ms(10);

    // Initialize the BH1750.
    uint8_t cmd = BH1750_CONT_H_RES_MODE;
    i2c_write_blocking(i2c0, LIGHT_SENSOR_I2C_ADDRESS, &cmd, 1, false);
    sleep_ms(180); // Wait for the first measurement (~120-180ms for high res).


    while (1) {
        sensor_readings.temperature = NAN;
        sensor_readings.humidity = NAN;
        sensor_readings.light_intensity = NAN;
        sensor_readings.air_pressure = NAN;
        sensor_readings.air_quality_index = NAN;

        if (read_temperature_and_humidity(&sensor_readings) == -1) {
            printf("Error when reading the temperature and humidity sensor. Error number %d\n",
                BME680_READ_ERROR);
        }
        else {
            printf("DHT22 readings: Temperature %.1fC, Humidity %.1f%%\n",
                   sensor_readings.temperature, sensor_readings.humidity);
        }

        if (read_bme680_sensor(bme680_sensor, bme680_conf, bme680_heater_conf, &sensor_readings) == -1) {
            printf("Error when reading the BME680 sensor measurements. Error number %d\n",
                BME680_READ_ERROR);
        }
        else {
            printf("Temperature %.2f, Humidity %.2f, Air pressure %.2f, Gas resistance (ohm) %d\n", sensor_readings.temperature, 
                sensor_readings.humidity, sensor_readings.air_pressure, 
                sensor_readings.air_quality_index);
        }

        if (read_light_intensity(&sensor_readings) == -1) {
            printf("Error when reading the light intensity sensor. Error number %d\n",
                LIGHT_SENSOR_READ_ERROR);
        }
        else {
            printf("Light intensity: %.2f\n", sensor_readings.light_intensity);
        }

        for (int i = 0; i < LENGTH_PREVIOUS_READINGS_ARRAY - 1; i++) {
            previous_readings[i] = previous_readings[i + 1];
        }
        previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY - 1] = sensor_readings;

        int hazard_code = analyze_hazards(previous_readings);
        if (hazard_code != 0) {
            printf("Potential hazard detected! Code: %d\n", hazard_code);
            activate_hazard_alert(hazard_code);
        }

        encrypt_ambient_info_message(
            sensor_readings, aes_ctx, AES_256_KEY, AES_256_IV, radio_message);

        if (transmit_radio_message(nrf24_module, radio_message) == -1) {
            printf("Error when sending the sensor readings. Error number %d\n", 
                DATA_TRANSMIT_ERROR);
        }
        else {
            printf("Packet transmitted successfully.\n");
        }
        printf("\n");


        printf("\n");
        // Wait another full minute before reading sensors again.
        sleep_ms(1000); //sleep_ms(MINUTE_IN_MILLISECONDS)
    }

}
