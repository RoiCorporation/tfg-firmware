#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "mongoose.h"
#include "central_station_firmware.h"
#include "errors.h"
#include "network.h"


int main() {

    // NRF24L01 module driver.
    nrf_client_t nrf24_module;

    // AES decryption engine context.
    struct AES_ctx aes_ctx;
    
    ambient_info_t wireless_station_info;

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
    // Maintain MQTT connection
    mg_timer_add(&connection_manager, 3000, MG_TIMER_REPEAT, mqtt_timer_fn, &network_context);
    handshake(nrf24_module, network_context.environmental_readings.station_id);
    
    while (1) {
        network_context.environmental_readings.temperature = NAN;
        network_context.environmental_readings.humidity = NAN;
        network_context.environmental_readings.light_intensity = NAN;
        network_context.environmental_readings.air_pressure = NAN;
        network_context.environmental_readings.air_quality_index = NAN;

        if (receive_radio_message(nrf24_module, radio_message) == -1) {
            printf("Error when receiving the wireless station readings. Error number %d\n", 
                DATA_RECEIVE_ERROR);
        }
        else {
            decrypt_ambient_info_message(
                &network_context.environmental_readings, aes_ctx, AES_256_KEY, 
                AES_256_IV, radio_message
            );

            printf("Temperature: %f\n",
                network_context.environmental_readings.temperature);
            printf("Humidity: %f\n",
                network_context.environmental_readings.humidity);
            printf("Light intensity: %f\n",
                network_context.environmental_readings.light_intensity);
            printf("Pressure: %f\n",
                network_context.environmental_readings.air_pressure);
            printf("Air quality index: %f\n",
                network_context.environmental_readings.air_quality_index);
            publish_environmental_readings(
                network_context.mqtt_connection,
                network_context.environmental_readings
            );
        }
        printf("\n");
        mg_mgr_poll(&connection_manager, 1000);

        sleep_ms(1000);
    }

    mg_mgr_free(&connection_manager);
    return 0;
}


// // mosquitto_pub -d -q 1 -h mqtt.thingsboard.cloud -p 1883 -t v1/devices/me/telemetry -i "Centralstation0001" -u "Centralstation0001" -P "Centralstation0001" -m "{temperature:25}"
// // To use mqtts, download the certificate here:
// // curl -f -S -o tb-cloud-root-ca.pem https://thingsboard.cloud/api/device-connectivity/mqtts/certificate/download
