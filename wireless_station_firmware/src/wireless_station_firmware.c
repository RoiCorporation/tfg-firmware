#include <stdio.h>
#include <math.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/sync.h"
#include "hardware/gpio.h"
#include "hardware/powman.h"
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
volatile int8_t has_associated_central_station;
uint32_t aes_ctr_counter;
volatile button_action_t button_action;


int main() {

    /* Variables */
    absolute_time_t next_minute_run;

    // ECDH variables.
    uint8_t ecdh_private_key[ECC_PRV_KEY_SIZE];
    uint8_t ecdh_public_key[ECC_PUB_KEY_SIZE];

    /* Components and module drivers */
    // BME680 sensor driver and its configuration and heater structures.
    struct bme68x_dev bme680_sensor;
    struct bme68x_conf bme680_conf;
    struct bme68x_heatr_conf bme680_heater_conf;

    // OLED display driver.
    ssd1306_t oled_display;
    
    // NRF24L01 module driver.
    nrf_client_t nrf24_module;

    // AES encryption engine.
    struct AES_ctx aes_ctx;

    uint8_t radio_message[sizeof(ambient_info_t) + AES_IV_COUNTER_SIZE];

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
        ecdh_private_key,
        ecdh_public_key,
        COPI_PIN,
        CIPO_PIN,
        SCK_PIN,
        CS_PIN,
        CE_PIN,
        SPI_BAUDRATE
    );
    sleep_ms(1500);
    printf("STARTED MAIN\n");

    // Initialize the context structures of both callbacks.
    display_timer_ctx = (display_timer_ctx_t){
        .display_turn = 1,
        .turns_until_display_off = 2 * AMBIENT_INFO_FIELD_COUNT,
        .oled_display = &oled_display,
        .station_readings = &station_readings
    };

    button_ctx = (button_ctx_t){
        .oled_display = &oled_display
    };

    while (1) {

        station_readings.temperature = NAN;
        station_readings.humidity = NAN;
        station_readings.light_intensity = NAN;
        station_readings.air_pressure = NAN;
        station_readings.air_quality_index = NAN;

        if (read_bme680_sensor(
            &bme680_sensor,
            &bme680_conf,
            &bme680_heater_conf,
            &station_readings
        ) == -1) {
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

        // Check if the button has been pressed.
        if (button_action != NO_ACTION) {

            // If it was a short press, start the OLED display sequence by
            // restarting the timer that controls it and resetting its control
            // variables.
            if (button_action == TURN_ON_DISPLAY) {
                cancel_repeating_timer(&display_turn_change_timer);

                display_timer_ctx.display_turn = 1;
                display_timer_ctx.turns_until_display_off = 2 * AMBIENT_INFO_FIELD_COUNT;
                ssd1306_poweron(display_timer_ctx.oled_display);
                display_turn_timer_callback(&display_turn_change_timer);
                add_repeating_timer_ms(
                    3000,
                    display_turn_timer_callback,
                    NULL,
                    &display_turn_change_timer);
            }

            // If it was a long press, invoke the handshake protocol to attempt
            // to associate this station with a central one.
            else if (button_action == START_HANDSHAKE) {
                cancel_repeating_timer(&display_turn_change_timer);
                ssd1306_poweron(&oled_display);
                ssd1306_draw_string(&oled_display, 0, 0, 1, "Connecting to station");
                ssd1306_show(&oled_display);
                
                // Start the handshake protocol.
                int8_t handshake_result = handshake(
                    &nrf24_module,
                    ecdh_private_key,
                    ecdh_public_key,
                    KDF_SALT,
                    &aes_ctx,
                    AES_256_IV
                );

                ssd1306_clear(&oled_display);

                // If the handshake procedure was successful, show an
                // informative message on the display. Also update the
                // variable that stores whether the station is associated
                // with a central station.
                if (handshake_result == 0) {
                    ssd1306_draw_string(&oled_display, 0, 16, 1, "Station associated");
                    ssd1306_draw_string(&oled_display, 0, 32, 1, "correctly with the");
                    ssd1306_draw_string(&oled_display, 0, 48, 1, "central station!");
                    ssd1306_show(&oled_display);
                    has_associated_central_station = 0;
                }

                // If the handshake procedure wasn't successful, show an
                // error message on the display.
                else {
                    printf("Error on the handshake when associating a wireless station. Error number %d\n",
                        HANDSHAKE_ERROR);

                    ssd1306_draw_string(&oled_display, 0, 16, 1, "An error occurred");
                    ssd1306_draw_string(&oled_display, 0, 32, 1, "trying to connect to");
                    ssd1306_draw_string(&oled_display, 0, 48, 1, "a central station.");
                    ssd1306_show(&oled_display);
                }
                sleep_ms(3000);

                // Restart the display timer.
                display_timer_ctx.display_turn = 1;
                display_timer_ctx.turns_until_display_off = 2 * AMBIENT_INFO_FIELD_COUNT;
                add_repeating_timer_ms(
                    3000,
                    display_turn_timer_callback,
                    NULL,
                    &display_turn_change_timer
                );
            }

            button_action = NO_ACTION;
        }

        // If the station is associated with a central one, encrypt and transmit
        // its sensor readings to it.
        if (has_associated_central_station == 0) {
            encrypt_ambient_info_message(
                &station_readings,
                &aes_ctx,
                AES_256_IV,
                radio_message
            );

            if (transmit_station_readings(
                &nrf24_module,
                radio_message,
                sizeof(radio_message)
            ) == -1) {
                printf("Error when sending the sensor readings. Error number %d\n", 
                    DATA_TRANSMIT_ERROR);
            }
            else
                printf("Packet transmitted successfully.\n");
        }

        // Wait for any interrupt caused by the touch button.
        while (button_action == NO_ACTION)
            __wfi();
    }

}
