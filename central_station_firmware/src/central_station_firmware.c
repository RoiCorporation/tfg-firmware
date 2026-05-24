#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "mongoose.h"
#include "network.h"
#include "ssd1306.h"
#include "oled_display.h"
#include "central_station_firmware.h"
#include "callbacks.h"
#include "hazards.h"
#include "alerts.h"
#include "errors.h"
#include "mq_sensors.h"


volatile button_action_t button_action = NO_ACTION;
volatile uint8_t display_turn_update = 0;
absolute_time_t time_button_press, time_button_release;


int main() {

    /* Variables */
    ambient_info_t station_readings, wireless_station_readings_buffer[NRF24_ADDRESSES_BUFFER_SIZE];
    ambient_info_t previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY];
    associated_wireless_station_info_t associated_wireless_stations_info_map[NRF24_ADDRESSES_BUFFER_SIZE];
    uint8_t radio_message[sizeof(float) * WIRELESS_STATION_DATA_FIELD_COUNT + AES_IV_COUNTER_SIZE];
    uint8_t incoming_packet_data_pipe,
        polls_until_publishing_readings = 0, mongoose_polling_enabled = 0;

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

    // Timer to manage the display turns.
    struct repeating_timer display_turn_change_timer;

    // Network stack context and its associated structures.
    struct mg_mgr connection_manager;
    network_ctx_t network_context = {0};
    network_context.connection_manager = &connection_manager;

    // Configure all the protocols, devices and pins in the station.
    initialize_station(
        &bme680_sensor,
        &bme680_conf,
        &bme680_heater_conf,
        &oled_display,
        &nrf24_module,
        associated_wireless_stations_info_map,
        ecdh_private_key,
        ecdh_public_key,
        network_context.connection_manager,
        COPI_PIN,
        CIPO_PIN,
        SCK_PIN,
        CS_PIN,
        CE_PIN,
        SPI_BAUDRATE
    );
    sleep_ms(200);

    // Initialize the elements of the array of previous readings
    // with their default values.
    for (int i = 0; i < LENGTH_PREVIOUS_READINGS_ARRAY; i++) {
        previous_readings[i].temperature = 0;
        previous_readings[i].humidity = 0;
        previous_readings[i].light_intensity = 0;
        previous_readings[i].air_pressure = 0;
        previous_readings[i].air_quality_index = 0;
        previous_readings[i].carbon_monoxide_concentration = 0;
        previous_readings[i].methane_concentration = 0;
        previous_readings[i].propane_concentration = 0;
        previous_readings[i].alcohol_concentration = 0;
        previous_readings[i].hydrogen_gas_concentration = 0;
    }

    // Initialize the context for the display turn timer.
    display_timer_ctx_t display_timer_ctx = {
        .display_turn = 1,
        .turns_until_display_off = 2 * AMBIENT_INFO_FIELD_COUNT
    };

    add_repeating_timer_ms(
        3000,
        display_turn_timer_callback,
        &display_timer_ctx,
        &display_turn_change_timer
    );

    for (int i = 0; i < NRF24_ADDRESSES_BUFFER_SIZE; i++) {
        printf("%d-th address: ", i);
        for (int j = 0; j < NRF24_ADDRESS_SIZE; j++) {
            printf("%x, ", associated_wireless_stations_info_map[i].nrf24l01_address[j]);
        }
        printf("\n");
    }

    ssd1306_draw_string(&oled_display, 0, 0, 1, "Connecting to WiFi...");
    ssd1306_show(&oled_display);

    mg_timer_add(&connection_manager, 200, MG_TIMER_REPEAT, mqtt_timer_fn, &network_context);

    while (mqtt_is_ready(&network_context) != 0)
        mg_mgr_poll(&connection_manager, 200);
    
    while (1) {

        // Check if the display should be turned on.
        if (button_action == TURN_ON_DISPLAY) {
            button_action = NO_ACTION;
            
            if (display_timer_ctx.turns_until_display_off == 0) {
                display_timer_ctx.display_turn = 1;
                display_timer_ctx.turns_until_display_off = 2 * AMBIENT_INFO_FIELD_COUNT;
                ssd1306_poweron(&oled_display);
                add_repeating_timer_ms(
                    3000,
                    display_turn_timer_callback,
                    &display_timer_ctx,
                    &display_turn_change_timer);
            }
        }

        // Check for incoming connections from wireless stations.
        else if (button_action == START_HANDSHAKE) {
            button_action = NO_ACTION;

            // Temporarily disable mongoose polling.
            mongoose_polling_enabled = -1;
            
            // Start the handshake procedure.
            int8_t handshake_result = handshake(
                &nrf24_module,
                ecdh_private_key,
                ecdh_public_key,
                KDF_SALT,
                AES_256_IV,
                associated_wireless_stations_info_map,
                NRF24_ADDRESSES_BUFFER_SIZE
            );

            // Reactivate mongoose polling.
            mongoose_polling_enabled = 0;

            printf("Map buffer state: \n");
            for (int i = 0; i < NRF24_ADDRESSES_BUFFER_SIZE; i++) {
                printf("%d: IDs: %s, counter: %u\n", i, 
                    associated_wireless_stations_info_map[i].station_id,
                    associated_wireless_stations_info_map[i].aes_ctr_counter
                );
            }

            // If the handshake procedure failed, print an appropriate error message.
            if (handshake_result != 0) {
                printf("Error on the handshake when associating a wireless station. Error number %d\n",
                    HANDSHAKE_ERROR);
            }
        }

        // Take the readings of this station once every minute. To calculate
        // that period, use the poll counter and the delay between polls.
        if (polls_until_publishing_readings == (3000 / MAIN_LOOP_POLLING_PERIOD_MS)) {
            
            // Reset the values inside the station readings struct.
            memcpy(station_readings.station_id, STATION_ID, STATION_ID_CHAR_LENGTH);
            station_readings.station_id[STATION_ID_CHAR_LENGTH - 1] = '\0';
            station_readings.temperature = 0;
            station_readings.humidity = 0;
            station_readings.light_intensity = 0;
            station_readings.air_pressure = 0;
            station_readings.air_quality_index = 0;
            station_readings.carbon_monoxide_concentration = 0;
            station_readings.methane_concentration = 0;
            station_readings.propane_concentration = 0;
            station_readings.alcohol_concentration = 0;
            station_readings.hydrogen_gas_concentration = 0;

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
                printf("Temperature %.2f, Humidity %.2f, Air pressure %.2f, IAQ %f\n",
                    station_readings.temperature,
                    station_readings.humidity,
                    station_readings.air_pressure,
                    station_readings.air_quality_index
                );
            }

            if (read_light_intensity(&station_readings) == -1) {
                printf("Error when reading the light intensity sensor. Error number %d\n",
                    LIGHT_SENSOR_READ_ERROR);
            }
            else {
                printf("Light intensity: %.2f\n", station_readings.light_intensity);
            }

            station_readings.carbon_monoxide_concentration = read_mq_7_co_ppm();
            station_readings.methane_concentration = read_mq_4_ch4_ppm();
            station_readings.propane_concentration = read_mq_6_c3h8_ppm();
            if (gpio_get(MQ_8_PIN))
                station_readings.hydrogen_gas_concentration = 1000;
            else
                station_readings.hydrogen_gas_concentration = 0;

            // Update the array of previous readings.
            for (int i = 0; i < LENGTH_PREVIOUS_READINGS_ARRAY - 1; i++) {
                previous_readings[i] = previous_readings[i + 1];
            }
            previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY - 1] = station_readings;

            // Analyze the previous readings to check if there are any hazards.
            int hazard_code = analyze_hazards(previous_readings);
            if (hazard_code != 0) {
                printf("Potential hazard detected! Code: %d\n", hazard_code);
                activate_hazard_alert(hazard_code);
            }

            // Publish the station readings iva MQTT.
            if (mqtt_is_ready(&network_context) == 0) {
                publish_environmental_readings(
                    network_context.mqtt_connection,
                    &station_readings
                );
            }
            polls_until_publishing_readings = 0;
        }

        // Check if the display turn callback has been fired. If it has, update
        // the display with the environmental reading that corresponds to the
        // new turn.
        if (display_turn_update != 0) {
            display_turn_update = 0;

            if (display_timer_ctx.turns_until_display_off > 0) {
                handle_display_turn_update(
                    display_timer_ctx.display_turn,
                    &oled_display,
                    &station_readings
                );
            }
            else {
                cancel_repeating_timer(&display_turn_change_timer);
                ssd1306_clear(&oled_display);
                ssd1306_show(&oled_display);
                ssd1306_poweroff(&oled_display);
            }
        }

        // Check if there is any message received by the radio.
        if (receive_station_readings(
            &nrf24_module,
            radio_message,
            sizeof(radio_message),
            &incoming_packet_data_pipe
        ) == -1) {
            printf("Error when receiving a wireless station readings. Error number %d\n", 
                DATA_RECEIVE_ERROR);
        }
        else {
            if (incoming_packet_data_pipe >= NRF24_ADDRESSES_BUFFER_SIZE) {
                printf("INVALID incoming_packet_data_pipe: %u\n", incoming_packet_data_pipe);
                continue;
            }

            // Reset the value of the ambient_info_t structure that corresponds
            // with the station that's in the position of the associated stations
            // info buffer of the data pipe where the message was received.
            wireless_station_readings_buffer[incoming_packet_data_pipe].temperature = NAN;
            wireless_station_readings_buffer[incoming_packet_data_pipe].humidity = NAN;
            wireless_station_readings_buffer[incoming_packet_data_pipe].light_intensity = NAN;
            wireless_station_readings_buffer[incoming_packet_data_pipe].air_pressure = NAN;
            wireless_station_readings_buffer[incoming_packet_data_pipe].air_quality_index = NAN;
            wireless_station_readings_buffer[incoming_packet_data_pipe].carbon_monoxide_concentration = NAN;
            wireless_station_readings_buffer[incoming_packet_data_pipe].methane_concentration = NAN;
            wireless_station_readings_buffer[incoming_packet_data_pipe].propane_concentration = NAN;
            wireless_station_readings_buffer[incoming_packet_data_pipe].alcohol_concentration = NAN;
            wireless_station_readings_buffer[incoming_packet_data_pipe].hydrogen_gas_concentration = NAN;

            // Decrypt the message received.
            decrypt_ambient_info_message(
                &(wireless_station_readings_buffer[incoming_packet_data_pipe]),
                &associated_wireless_stations_info_map[incoming_packet_data_pipe].aes_ctx,
                AES_256_IV,
                radio_message
            );

            // Copy the station's ID that's saved in that stations's position of
            // the associated stations' info buffer into the receiving info
            // struct's station_id field.
            memcpy(
                wireless_station_readings_buffer[incoming_packet_data_pipe].station_id,
                associated_wireless_stations_info_map[incoming_packet_data_pipe].station_id,
                STATION_ID_CHAR_LENGTH
            );
            wireless_station_readings_buffer[
                incoming_packet_data_pipe].station_id[STATION_ID_CHAR_LENGTH - 1] = '\0';

            printf("Temperature: %f; Humidity: %f; Light intensity: %f; Air pressure: %f\n",
                wireless_station_readings_buffer[incoming_packet_data_pipe].temperature,
                wireless_station_readings_buffer[incoming_packet_data_pipe].humidity,
                wireless_station_readings_buffer[incoming_packet_data_pipe].light_intensity,
                wireless_station_readings_buffer[incoming_packet_data_pipe].air_pressure
            );

            // Publish the readings received from the associated station via MQTT.
            if (mqtt_is_ready(&network_context) == 0) {
                publish_environmental_readings(
                    network_context.mqtt_connection,
                    &wireless_station_readings_buffer[incoming_packet_data_pipe]
                );
            }
        }

        polls_until_publishing_readings++;
        sleep_ms(MAIN_LOOP_POLLING_PERIOD_MS);
        printf("\n");
        if (mongoose_polling_enabled == 0)
            mg_mgr_poll(&connection_manager, 10);
    }

    mg_mgr_free(&connection_manager);
    return 0;
}


// // // mosquitto_pub -d -q 1 -h mqtt.thingsboard.cloud -p 1883 -t v1/devices/me/telemetry -i "Centralstation0001" -u "Centralstation0001" -P "Centralstation0001" -m "{temperature:25}"
// // // To use mqtts, download the certificate here:
// // // curl -f -S -o tb-cloud-root-ca.pem https://thingsboard.cloud/api/device-connectivity/mqtts/certificate/download
