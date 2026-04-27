#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "oled_display.h"
#include "wireless_station_firmware.h"
#include "callbacks.h"
#include "hazards.h"
#include "alerts.h"
#include "errors.h"


absolute_time_t time_button_press, time_button_release;
struct repeating_timer display_turn_change_timer;
button_ctx_t button_ctx;
display_timer_ctx_t display_timer_ctx;


int main() {

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

    uint8_t radio_message[sizeof(ambient_info_t)];

    // Pointer to a ambient_info_t struct that stores all the data
    // read by the sensors.
    ambient_info_t station_readings;
    
    // Array of ambient_info_t elements that holds the n-previous
    // sensor readings. It's used to check for potential upcoming 
    // hazards, such as a flood, a sudden fire or a gas leak.
    ambient_info_t previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY];

    // Initialize the values in the array of previous measurements.
    for (int i = 0; i < LENGTH_PREVIOUS_READINGS_ARRAY; i++) {
        previous_readings[i].temperature = 0;
        previous_readings[i].humidity = 0;
        previous_readings[i].light_intensity = 0;
        previous_readings[i].air_pressure = 0;
        previous_readings[i].air_quality_index = 0;
    }

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
    sleep_ms(200);

    display_timer_ctx = (display_timer_ctx_t){
        .display_turn = 1,
        .turns_until_display_off = 2 * AMBIENT_INFO_FIELD_COUNT,
        .oled_display = &oled_display,
        .station_readings = &station_readings
    };

    button_ctx = (button_ctx_t){
        .oled_display = &oled_display,
        .nrf24_module = &nrf24_module
    };

    add_repeating_timer_ms(
        3000,
        display_turn_timer_callback,
        NULL,
        &display_turn_change_timer
    );

    while (1) {

        tight_loop_contents();

        station_readings.temperature = 0;
        station_readings.humidity = 0;
        station_readings.light_intensity = 0;
        station_readings.air_pressure = 0;
        station_readings.air_quality_index = 0;

        if (read_bme680_sensor(bme680_sensor, bme680_conf, bme680_heater_conf, &station_readings) == -1) {
            printf("Error when reading the BME680 sensor measurements. Error number %d\n",
                BME680_READ_ERROR);
        }
        else {
            printf("Temperature %.2f, Humidity %.2f, Air pressure %.2f, AQI (ohm) %f\n", station_readings.temperature, 
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

        //TODO: change this when we find a way to actually calculate the correct AQI.
        station_readings.air_quality_index = 80;

        for (int i = 0; i < LENGTH_PREVIOUS_READINGS_ARRAY - 1; i++) {
            previous_readings[i] = previous_readings[i + 1];
        }
        previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY - 1] = station_readings;

        int hazard_code = analyze_hazards(previous_readings);
        if (hazard_code != 0) {
            printf("Potential hazard detected! Code: %d\n", hazard_code);
            activate_hazard_alert(hazard_code);
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
        sleep_ms(MINUTE_IN_MILLISECONDS);
    }

}
