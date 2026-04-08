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
#include "central_station_firmware.h"
#include "hazards.h"
#include "alerts.h"
#include "errors.h"


queue_t call_queue;


void core1_entry() {

    queue_entry_t call_queue_entry;

    queue_remove_blocking(&call_queue, &call_queue_entry);
    
    // Array of ambient_info_t elements that holds the n-previous
    // sensor readings. It's used to check for potential upcoming 
    // hazards, such as a flood, a sudden fire or a gas leak.
    ambient_info_t previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY];
    
    ambient_info_t *station_readings = &call_queue_entry.network_context.environmental_readings;

    while (1) {
        memcpy(station_readings->station_id, STATION_ID, STATION_ID_CHAR_LENGTH);
        station_readings->station_id[STATION_ID_CHAR_LENGTH - 1] = '\0';
        station_readings->temperature = 55.2;
        station_readings->humidity = 3.2;
        station_readings->light_intensity = NAN;
        station_readings->air_pressure = NAN;
        station_readings->air_quality_index = NAN;
        station_readings->carbon_monoxide_concentration = NAN;
        station_readings->methane_concentration = NAN;
        station_readings->propane_concentration = NAN;
        station_readings->alcohol_concentration = NAN;
        station_readings->hydrogen_gas_concentration = NAN;

        // if (read_temperature_and_humidity(&station_readings) == -1) {
        //     printf("Error when reading the temperature and humidity sensor. Error number %d\n",
        //         BME680_READ_ERROR);
        // }
        // else {
        //     printf("DHT22 readings: Temperature %.1fC, Humidity %.1f%%\n",
        //            station_readings.temperature, station_readings.humidity);
        // }

        // if (read_bme680_sensor(
        //     call_queue_entry.bme680_sensor,
        //     call_queue_entry.bme680_conf,
        //     call_queue_entry.bme680_heater_conf,
        //     &station_readings) == -1
        // ) {
        //     printf("Error when reading the BME680 sensor measurements. Error number %d\n",
        //         BME680_READ_ERROR);
        // }
        // else {
        //     printf("Temperature %.2f, Humidity %.2f, Air pressure %.2f, Gas resistance (ohm) %d\n", station_readings.temperature, 
        //         station_readings.humidity, station_readings.air_pressure, 
        //         station_readings.air_quality_index);
        // }

        // if (read_light_intensity(&station_readings) == -1) {
        //     printf("Error when reading the light intensity sensor. Error number %d\n",
        //         LIGHT_SENSOR_READ_ERROR);
        // }
        // else {
        //     printf("Light intensity: %.2f\n", station_readings.light_intensity);
        // }

        // for (int i = 0; i < LENGTH_PREVIOUS_READINGS_ARRAY - 1; i++) {
        //     previous_readings[i] = previous_readings[i + 1];
        // }
        // previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY - 1] = station_readings;

        // int hazard_code = analyze_hazards(previous_readings);
        // if (hazard_code != 0) {
        //     printf("Potential hazard detected! Code: %d\n", hazard_code);
        //     activate_hazard_alert(hazard_code);
        // }

        if (mqtt_is_ready(&call_queue_entry.network_context) == 0) {
            publish_environmental_readings(
                call_queue_entry.network_context.mqtt_connection,
                *station_readings
            );
        }
        
        mg_mgr_poll(call_queue_entry.network_context.connection_manager, 1000);
        
        // Wait another full minute before reading sensors again.
        sleep_ms(1000); //sleep_ms(MINUTE_IN_MILLISECONDS)

    }
}


int main() {

    // NRF24L01 module driver.
    nrf_client_t nrf24_module;

    // AES decryption engine context.
    struct AES_ctx aes_ctx;
    
    ambient_info_t station_readings, wireless_station_info;

    // BME680 sensor driver and its configuration and heater structures.
    struct bme68x_dev bme680_sensor;
    struct bme68x_conf bme680_conf;
    struct bme68x_heatr_conf bme680_heater_conf;

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

    uint8_t radio_message[sizeof(ambient_info_t)];

    // Configure all the protocols, devices and pins in the station.
    initialize_station(
        &bme680_sensor,
        &bme680_conf,
        &bme680_heater_conf,
        &nrf24_module,
        network_context.connection_manager,
        COPI_PIN,
        CIPO_PIN,
        SCK_PIN,
        CS_PIN,
        CE_PIN,
        SPI_BAUDRATE
    );

    while (!stdio_usb_connected()) sleep_ms(10);
    mg_timer_add(&connection_manager, 3000, MG_TIMER_REPEAT, mqtt_timer_fn, &network_context);

    while (mqtt_is_ready(&network_context) != 0) {
        mg_mgr_poll(&connection_manager, 1000);
    }

    queue_init(&call_queue, sizeof(queue_entry_t), 1);
    
    queue_entry_t call_queue_entry = {
        .bme680_sensor = bme680_sensor,
        .bme680_conf = bme680_conf,
        .bme680_heater_conf = bme680_heater_conf,
        .network_context = network_context
    };

    multicore_launch_core1(core1_entry);
    queue_add_blocking(&call_queue, &call_queue_entry);

    // Maintain MQTT connection
    handshake(nrf24_module, wireless_station_info.station_id);
    
    while (1) {
        wireless_station_info.temperature = NAN;
        wireless_station_info.humidity = NAN;
        wireless_station_info.light_intensity = NAN;
        wireless_station_info.air_pressure = NAN;
        wireless_station_info.air_quality_index = NAN;

        if (receive_radio_message(nrf24_module, radio_message) == -1) {
            printf("Error when receiving the wireless station readings. Error number %d\n", 
                DATA_RECEIVE_ERROR);
        }
        else {
            decrypt_ambient_info_message(
                &wireless_station_info, aes_ctx, AES_256_KEY,
                AES_256_IV, radio_message
            );

            if (mqtt_is_ready(&network_context) == 0) {
                publish_environmental_readings(
                    network_context.mqtt_connection,
                    wireless_station_info
                );
            }
        }
        printf("\n");

        mg_mgr_poll(&connection_manager, 1000);
        sleep_ms(1000);
    }

    mg_mgr_free(&connection_manager);
    return 0;
}


// // // mosquitto_pub -d -q 1 -h mqtt.thingsboard.cloud -p 1883 -t v1/devices/me/telemetry -i "Centralstation0001" -u "Centralstation0001" -P "Centralstation0001" -m "{temperature:25}"
// // // To use mqtts, download the certificate here:
// // // curl -f -S -o tb-cloud-root-ca.pem https://thingsboard.cloud/api/device-connectivity/mqtts/certificate/download

