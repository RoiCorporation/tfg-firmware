#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "pico/util/queue.h"
#include "pico/multicore.h"
#include "mongoose.h"
#include "network.h"
#include "ssd1306.h"
#include "oled_display.h"
#include "central_station_firmware.h"
#include "hazards.h"
#include "alerts.h"
#include "errors.h"


queue_t call_queue;

bool display_reading_timer_callback(__unused struct repeating_timer *t) {
    uint8_t *display_turn = (uint8_t *)t->user_data;
    if (*display_turn == 9)
        *display_turn = 0;
    else
        (*display_turn)++;
    return true;
}

void core1_entry() {
    
    // Queue entry that contains all the necessary structures and drivers
    // to display and upload the station' sensor readings.
    queue_entry_t call_queue_entry;
    queue_remove_blocking(&call_queue, &call_queue_entry);

    // Array of ambient_info_t elements that holds the n-previous
    // sensor readings. It's used to check for potential upcoming 
    // hazards, such as a flood, a sudden fire or a gas leak.
    ambient_info_t previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY];
    ambient_info_t station_readings;
    
    struct repeating_timer timer;
    uint8_t display_turn = 0;
 
    add_repeating_timer_ms(3000, display_reading_timer_callback, &display_turn, &timer);

    while (1) {

        tight_loop_contents();

        // Initialize the values inside the station readings struct.
        memcpy(station_readings.station_id, STATION_ID, STATION_ID_CHAR_LENGTH);
        station_readings.station_id[STATION_ID_CHAR_LENGTH - 1] = '\0';
        station_readings.temperature = NAN;
        station_readings.humidity = NAN;
        station_readings.light_intensity = NAN;
        station_readings.air_pressure = NAN;
        station_readings.air_quality_index = NAN;
        station_readings.carbon_monoxide_concentration = NAN;
        station_readings.methane_concentration = NAN;
        station_readings.propane_concentration = NAN;
        station_readings.alcohol_concentration = NAN;
        station_readings.hydrogen_gas_concentration = NAN;

        // if (read_temperature_and_humidity(&station_readings) == -1) {
        //     printf("Error when reading the temperature and humidity sensor. Error number %d\n",
        //         BME680_READ_ERROR);
        // }
        // else {
        //     printf("DHT22 readings: Temperature %.1fC, Humidity %.1f%%\n",
        //            station_readings.temperature, station_readings.humidity);
        // }

        if (read_bme680_sensor(
            call_queue_entry.bme680_sensor,
            call_queue_entry.bme680_conf,
            call_queue_entry.bme680_heater_conf,
            &station_readings) == -1
        ) {
            printf("Error when reading the BME680 sensor measurements. Error number %d\n",
                BME680_READ_ERROR);
        }
        else {
            printf("Temperature %.2f, Humidity %.2f, Air pressure %.2f, Gas resistance (ohm) %d\n", station_readings.temperature, 
                station_readings.humidity, station_readings.air_pressure, 
                station_readings.air_quality_index);
        }

        // if (read_light_intensity(&station_readings) == -1) {
        //     printf("Error when reading the light intensity sensor. Error number %d\n",
        //         LIGHT_SENSOR_READ_ERROR);
        // }
        // else {
        //     printf("Light intensity: %.2f\n", station_readings.light_intensity);
        // }

        switch(display_turn) {
            case 0:
                display_temperature(
                    call_queue_entry.oled_display,
                    station_readings.temperature
                );
                break;
            case 1:
                display_humidity(
                    call_queue_entry.oled_display,
                    station_readings.humidity
                );
                break;
            case 2:
                display_light_intensity(
                    call_queue_entry.oled_display,
                    station_readings.light_intensity
                );
                break;
            case 3:
                display_air_pressure(
                    call_queue_entry.oled_display,
                    station_readings.air_pressure
                );
                break;
            case 4:
                display_air_quality_index(
                    call_queue_entry.oled_display,
                    station_readings.air_quality_index
                );
                break;
            case 5:
                display_carbon_monoxide_concentration(
                    call_queue_entry.oled_display,
                    station_readings.carbon_monoxide_concentration
                );
                break;
            case 6:
                display_methane_concentration(
                    call_queue_entry.oled_display,
                    station_readings.methane_concentration
                );
                break;
            case 7:
                display_propane_concentration(
                    call_queue_entry.oled_display,
                    station_readings.propane_concentration
                );
                break;
            case 8:
                display_alcohol_concentration(
                    call_queue_entry.oled_display,
                    station_readings.alcohol_concentration
                );
                break;
            case 9:
                display_hydrogen_gas_concentration(
                    call_queue_entry.oled_display,
                    station_readings.hydrogen_gas_concentration
                );
                break;
            default:
                break;
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

        // Update the OLED display with the new readings.

        // display_hydrogen_gas_concentration(call_queue_entry.oled_display, 512);
        // sleep_ms(2000);
        // display_hydrogen_gas_concentration(call_queue_entry.oled_display, 9177.123);
        // sleep_ms(2000);
        // display_hydrogen_gas_concentration(&oled_display, 8769);
        // sleep_ms(3000);
        // display_hydrogen_gas_concentration(&oled_display, 57252);
        // sleep_ms(3000);
        // display_hydrogen_gas_concentration(&oled_display, 10001.23);
        // sleep_ms(3000);
        // display_hydrogen_gas_concentration(&oled_display, 389);
        // sleep_ms(3000);
        // display_hydrogen_gas_concentration(&oled_display, 1333.528);
        // sleep_ms(3000);
        
        if (mqtt_is_ready(&call_queue_entry.network_context) == 0) {
            publish_environmental_readings(
                call_queue_entry.network_context.mqtt_connection,
                &station_readings
            );
        }
        
        mg_mgr_poll(call_queue_entry.network_context.connection_manager, 1000);
        
        // Wait another full minute before reading sensors again.
        sleep_ms(1000); //sleep_ms(MINUTE_IN_MILLISECONDS)

    }
}


int main() {

    // Data variables used throughout the program lifetime.
    ambient_info_t station_readings, wireless_station_readings_buffer[NRF24_ADDRESSES_BUFFER_SIZE];
    uint8_t incoming_packet_data_pipe;
    station_id_address_map_t station_id_to_nrf24_address_buffer[NRF24_ADDRESSES_BUFFER_SIZE];
    uint8_t radio_message[sizeof(ambient_info_t)];
    
    // BME680 sensor driver and its configuration and heater structures.
    struct bme68x_dev bme680_sensor;
    struct bme68x_conf bme680_conf;
    struct bme68x_heatr_conf bme680_heater_conf;
    
    // OLED display driver.
    ssd1306_t oled_display;

    // NRF24L01 module driver.
    nrf_client_t nrf24_module;    

    // AES decryption engine context.
    struct AES_ctx aes_ctx;

    // Network stack context and its associated structures.
    struct mg_mgr connection_manager;
    network_ctx_t network_context = {0};
    network_context.connection_manager = &connection_manager;
    network_context.environmental_readings = (ambient_info_t) {
        .temperature = NAN,
        .humidity = NAN,
        .light_intensity = NAN,
        .air_pressure = NAN,
        .air_quality_index = NAN,
        .carbon_monoxide_concentration = NAN,
        .methane_concentration = NAN,
        .propane_concentration = NAN,
        .alcohol_concentration = NAN,
        .hydrogen_gas_concentration = NAN
    };
    memcpy(network_context.environmental_readings.station_id, STATION_ID, STATION_ID_CHAR_LENGTH);
    network_context.environmental_readings.station_id[STATION_ID_CHAR_LENGTH - 1] = '\0';

    // Configure all the protocols, devices and pins in the station.
    initialize_station(
        &bme680_sensor,
        &bme680_conf,
        &bme680_heater_conf,
        &oled_display,
        &nrf24_module,
        station_id_to_nrf24_address_buffer,
        network_context.connection_manager,
        COPI_PIN,
        CIPO_PIN,
        SCK_PIN,
        CS_PIN,
        CE_PIN,
        SPI_BAUDRATE
    );

    for (int i = 0; i < NRF24_ADDRESSES_BUFFER_SIZE; i++) {
        printf("%d-th address: ", i);
        for (int j = 0; j < NRF24_ADDRESS_SIZE; j++) {
            printf("%x, ", station_id_to_nrf24_address_buffer[i].nrf24l01_address[j]);
        }
        printf("\n");
    }

    ssd1306_draw_string(&oled_display, 0, 0, 1, "Connecting to WiFi...");
    ssd1306_show(&oled_display);

    mg_timer_add(&connection_manager, 3000, MG_TIMER_REPEAT, mqtt_timer_fn, &network_context);

    while (mqtt_is_ready(&network_context) != 0) {
        mg_mgr_poll(&connection_manager, 1000);
    }

    queue_init(&call_queue, sizeof(queue_entry_t), 1);
    
    queue_entry_t call_queue_entry = {
        .bme680_sensor = bme680_sensor,
        .bme680_conf = bme680_conf,
        .bme680_heater_conf = bme680_heater_conf,
        .oled_display = &oled_display,
        .network_context = network_context
    };

    // Launch the other core to start reading values with the sensors 
    // of this station, displaying their measurements on the OLED display
    // and uploading them to the database.
    multicore_launch_core1(core1_entry);
    queue_add_blocking(&call_queue, &call_queue_entry);

    // Check for incoming connections from wireless stations.
    int8_t handshake_result = handshake(
        &nrf24_module,
        station_id_to_nrf24_address_buffer,
        NRF24_ADDRESSES_BUFFER_SIZE
    );
    if (handshake_result != 0) {
        printf("Error on the handshake when associating a wireless station. Error number %d\n",
            HANDSHAKE_ERROR);
    }
    
    else {

        while (1) {

            if (receive_radio_message(
                nrf24_module,
                radio_message,
                &incoming_packet_data_pipe
            ) == -1) {
                printf("Error when receiving a wireless station readings. Error number %d\n", 
                    DATA_RECEIVE_ERROR);
            }
            else {
                wireless_station_readings_buffer[incoming_packet_data_pipe].temperature = NAN;
                wireless_station_readings_buffer[incoming_packet_data_pipe].humidity = NAN;
                wireless_station_readings_buffer[incoming_packet_data_pipe].light_intensity = NAN;
                wireless_station_readings_buffer[incoming_packet_data_pipe].air_pressure = NAN;
                wireless_station_readings_buffer[incoming_packet_data_pipe].air_quality_index = NAN;

                decrypt_ambient_info_message(
                    &(wireless_station_readings_buffer[incoming_packet_data_pipe]),
                    aes_ctx, AES_256_KEY, AES_256_IV, radio_message
                );

                memcpy(
                    wireless_station_readings_buffer[incoming_packet_data_pipe].station_id,
                    station_id_to_nrf24_address_buffer[incoming_packet_data_pipe].associated_station_id,
                    STATION_ID_CHAR_LENGTH
                );
                wireless_station_readings_buffer[
                    incoming_packet_data_pipe].station_id[STATION_ID_CHAR_LENGTH - 1] = '\0';

                printf("Temperature: %f; Humidity: %f; ID: %s\n",
                    wireless_station_readings_buffer[incoming_packet_data_pipe].temperature,
                    wireless_station_readings_buffer[incoming_packet_data_pipe].humidity,
                    wireless_station_readings_buffer[incoming_packet_data_pipe].station_id
                );

                if (mqtt_is_ready(&network_context) == 0) {
                    publish_environmental_readings(
                        network_context.mqtt_connection,
                        &wireless_station_readings_buffer[incoming_packet_data_pipe]
                    );
                }
            }
            printf("\n");

            // mg_mgr_poll(&connection_manager, 1000);
            sleep_ms(1000);
        }
    }

    mg_mgr_free(&connection_manager);
    return 0;
}


// // // mosquitto_pub -d -q 1 -h mqtt.thingsboard.cloud -p 1883 -t v1/devices/me/telemetry -i "Centralstation0001" -u "Centralstation0001" -P "Centralstation0001" -m "{temperature:25}"
// // // To use mqtts, download the certificate here:
// // // curl -f -S -o tb-cloud-root-ca.pem https://thingsboard.cloud/api/device-connectivity/mqtts/certificate/download
