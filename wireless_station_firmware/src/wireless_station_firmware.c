#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "oled_display.h"
#include "wireless_station_firmware.h"
#include "hazards.h"
#include "alerts.h"
#include "errors.h"


bool display_reading_timer_callback(__unused struct repeating_timer *t) {
    uint8_t *display_turn = (uint8_t *)t->user_data;
    if (*display_turn == 4)
        *display_turn = 0;
    else
        (*display_turn)++;
    return true;
}


int main() {

    // Pointer to a ambient_info_t struct that stores all the data
    // read by the sensors.
    ambient_info_t station_readings;
    
    // Array of ambient_info_t elements that holds the n-previous
    // sensor readings. It's used to check for potential upcoming 
    // hazards, such as a flood, a sudden fire or a gas leak.
    ambient_info_t previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY];

    // BME680 sensor driver and its configuration and heater structures.
    struct bme68x_dev bme680_sensor;
    struct bme68x_conf bme680_conf;
    struct bme68x_heatr_conf bme680_heater_conf;

    // OLED display driver.
    ssd1306_t oled_display;
    
    // NRF24L01 module driver.
    nrf_client_t nrf24_module;

    // AES encryption engine context.
    struct AES_ctx aes_ctx;

    struct repeating_timer timer;
    uint8_t display_turn = 0;
    uint8_t radio_message[sizeof(ambient_info_t)];

    // Configure all the protocols, devices and pins in the station.
    initialize_station(
        &bme680_sensor,
        &bme680_conf,
        &bme680_heater_conf,
        &oled_display,
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
    sleep_ms(180); // Wait for the first measurement.

    add_repeating_timer_ms(3000, display_reading_timer_callback, &display_turn, &timer);

    if (handshake(&nrf24_module) != 0) {
        printf("Error on the handshake when associating a wireless station. Error number %d\n",
            HANDSHAKE_ERROR);
    }

    if (1 == 1) {

        while (1) {

            tight_loop_contents();

            station_readings.temperature = NAN;
            station_readings.humidity = NAN;
            station_readings.light_intensity = NAN;
            station_readings.air_pressure = NAN;
            station_readings.air_quality_index = NAN;

            if (read_temperature_and_humidity(&station_readings) == -1) {
                printf("Error when reading the temperature and humidity sensor. Error number %d\n",
                    BME680_READ_ERROR);
            }
            else {
                printf("DHT22 readings: Temperature %.1fC, Humidity %.1f%%\n",
                    station_readings.temperature, station_readings.humidity);
            }

            if (read_bme680_sensor(bme680_sensor, bme680_conf, bme680_heater_conf, &station_readings) == -1) {
                printf("Error when reading the BME680 sensor measurements. Error number %d\n",
                    BME680_READ_ERROR);
            }
            else {
                printf("Temperature %.2f, Humidity %.2f, Air pressure %.2f, Gas resistance (ohm) %d\n", station_readings.temperature, 
                    station_readings.humidity, station_readings.air_pressure, 
                    station_readings.air_quality_index);
            }

            if (read_light_intensity(&station_readings) == -1) {
                printf("Error when reading the light intensity sensor. Error number %d\n",
                    LIGHT_SENSOR_READ_ERROR);
            }
            else {
                printf("Light intensity: %.2f\n", station_readings.light_intensity);
            }

            for (int i = 0; i < LENGTH_PREVIOUS_READINGS_ARRAY - 1; i++) {
                previous_readings[i] = previous_readings[i + 1];
            }
            previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY - 1] = station_readings;

            int hazard_code = analyze_hazards(previous_readings);
            if (hazard_code != 0) {
                printf("Potential hazard detected! Code: %d\n", hazard_code);
                activate_hazard_alert(hazard_code);
            }

            switch(display_turn) {
                case 0:
                    display_temperature(
                        &oled_display,
                        station_readings.temperature
                    );
                    break;
                case 1:
                    display_humidity(
                        &oled_display,
                        station_readings.humidity
                    );
                    break;
                case 2:
                    display_light_intensity(
                        &oled_display,
                        station_readings.light_intensity
                    );
                    break;
                case 3:
                    display_air_pressure(
                        &oled_display,
                        station_readings.air_pressure
                    );
                    break;
                case 4:
                    display_air_quality_index(
                        &oled_display,
                        station_readings.air_quality_index
                    );
                    break;
                default:
                    break;
            }

            encrypt_ambient_info_message(
                station_readings, aes_ctx, AES_256_KEY, AES_256_IV, radio_message);

            if (transmit_radio_message(nrf24_module, radio_message) == -1) {
                printf("Error when sending the sensor readings. Error number %d\n", 
                    DATA_TRANSMIT_ERROR);
            }
            else {
                printf("Packet transmitted successfully.\n");
            }

            // Wait another full minute before reading sensors again.
            sleep_ms(1000); //sleep_ms(MINUTE_IN_MILLISECONDS)
        }
    }

}
