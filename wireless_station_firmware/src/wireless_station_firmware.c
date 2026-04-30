#include <stdio.h>
#include <math.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/i2c.h"
#include "hardware/sync.h"
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
volatile uint8_t button_event_pending;
volatile uint8_t minute_task_pending;
uint32_t aes_ctr_counter;
button_action_t button_action;


int main() {

    /* Variables */
    absolute_time_t next_minute_run;
    struct repeating_timer minute_timer;


    // ECDH and AES variables.
    uint8_t ecdh_private_key[ECC_PRV_KEY_SIZE];
    uint8_t ecdh_public_key[ECC_PUB_KEY_SIZE];
    uint8_t aes_key[AES_KEY_SIZE];

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
        ecdh_private_key,
        ecdh_public_key,
        COPI_PIN,
        CIPO_PIN,
        SCK_PIN,
        CS_PIN,
        CE_PIN,
        SPI_BAUDRATE
    );
    sleep_ms(1000);

    printf("Generated public key: \n");
    for (int i = 0; i < sizeof(ecdh_public_key); i++) {
        printf("%x\t", ecdh_public_key[i]);
        if (i % 8 == 0)
            printf("\n");
    }
    printf("\n");

    button_action = NO_ACTION;
    button_event_pending = 0;
    minute_task_pending = 0;
    aes_ctr_counter = 0;

    // Initialize the context structures of both callbacks.
    display_timer_ctx = (display_timer_ctx_t){
        .display_turn = 1,
        .turns_until_display_off = 2 * AMBIENT_INFO_FIELD_COUNT,
        .oled_display = &oled_display,
        .station_readings = &station_readings
    };

    button_ctx = (button_ctx_t){
        .oled_display = &oled_display
        // .nrf24_module = &nrf24_module,
        // .aes_ctx = &aes_ctx
    };
    // memcpy(button_ctx.ecdh_private_key, ecdh_private_key, ECC_PRV_KEY_SIZE);
    // memcpy(button_ctx.ecdh_public_key, ecdh_public_key, ECC_PUB_KEY_SIZE);
    // memcpy(button_ctx.aes_key, aes_key, AES_KEY_SIZE);
    // memcpy(button_ctx.aes_iv, AES_256_IV, AES_IV_SIZE);

    add_repeating_timer_ms(
        3000,
        display_turn_timer_callback,
        NULL,
        &display_turn_change_timer
    );

    add_repeating_timer_ms(
        60000,
        minute_timer_callback,
        NULL,
        &minute_timer
    );
    
    while (1) {

        if (button_event_pending) {
            button_event_pending = 0;
            printf("TREATED INTERRUPTION\n");

            // Whether it was a short or a long press, start the OLED display
            // sequence by restarting the timer that controls it and resetting
            // its control variables.
            cancel_repeating_timer(&display_turn_change_timer);

            display_timer_ctx.display_turn = 1;
            display_timer_ctx.turns_until_display_off = 2 * AMBIENT_INFO_FIELD_COUNT;
            ssd1306_poweron(display_timer_ctx.oled_display);
            add_repeating_timer_ms(
                3000,
                display_turn_timer_callback,
                NULL,
                &display_turn_change_timer
            );

            if (button_action == START_HANDSHAKE) {
                ssd1306_draw_string(&oled_display, 0, 0, 1, "Connecting to station");
                ssd1306_show(&oled_display);
                
                // Start the handshake procedure.
                int8_t handshake_result = handshake(
                    &nrf24_module,
                    ecdh_private_key,
                    ecdh_public_key,
                    KDF_SALT,
                    &aes_ctx,
                    aes_key,
                    AES_256_IV
                );

                // If the handshake procedure failed, print an appropriate error message.
                if (handshake_result != 0) {
                    printf("Error on the handshake when associating a wireless station. Error number %d\n",
                        HANDSHAKE_ERROR);
                    ssd1306_draw_string(button_ctx.oled_display, 0, 16, 1, "An error occurred");
                    ssd1306_draw_string(button_ctx.oled_display, 0, 32, 1, "trying to connect to");
                    ssd1306_draw_string(button_ctx.oled_display, 0, 48, 1, "a central station.");
                    ssd1306_show(button_ctx.oled_display);

                }
            }

            button_action = NO_ACTION;
        }

        if (minute_task_pending == 1) {

            minute_task_pending = 0;

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
                station_readings, &aes_ctx, aes_key, AES_256_IV, radio_message);

            if (transmit_radio_message(nrf24_module, radio_message) == -1) {
                printf("Error when sending the sensor readings. Error number %d\n", 
                    DATA_TRANSMIT_ERROR);
            }
            else {
                printf("Packet transmitted successfully.\n");
            }
        }

        // Wait another full minute before reading sensors again.
        __wfi();
    }

}
