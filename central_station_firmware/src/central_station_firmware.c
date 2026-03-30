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

    struct mg_mgr mgr;
    network_ctx_t net = {0};
    net.mgr = &mgr;
    net.environmental_readings = (ambient_info_t) {
        .station_id = "41fab1f5-f5e7-4494-abdc-941dba10d683",
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
        COPI_PIN,
        CIPO_PIN,
        SCK_PIN,
        CS_PIN,
        CE_PIN,
        SPI_BAUDRATE
    );

    while (!stdio_usb_connected()) sleep_ms(10);

    mg_mgr_init(&mgr);

    // Maintain MQTT connection
    mg_timer_add(&mgr, 3000, MG_TIMER_REPEAT, mqtt_timer_fn, &net);
    mg_timer_add(&mgr, 5000, MG_TIMER_REPEAT, sensor_readings_timer, &net);
    
    while (1) {
        net.environmental_readings.temperature = NAN;
        net.environmental_readings.humidity = NAN;
        net.environmental_readings.light_intensity = NAN;
        net.environmental_readings.air_pressure = NAN;
        net.environmental_readings.air_quality_index = NAN;

        if (receive_radio_message(nrf24_module, radio_message) == -1) {
            printf("Error when receiving the wireless station readings. Error number %d\n", 
                DATA_RECEIVE_ERROR);
        }
        else {
            decrypt_ambient_info_message(
                &net.environmental_readings, aes_ctx, AES_256_KEY, 
                AES_256_IV, radio_message
            );

            printf("Temperature: %f\n", net.environmental_readings.temperature);
            printf("Humidity: %f\n", net.environmental_readings.humidity);
            printf("Light intensity: %f\n", net.environmental_readings.light_intensity);
            printf("Pressure: %f\n", net.environmental_readings.air_pressure);
            printf("Air quality index: %f\n", net.environmental_readings.air_quality_index);
        }
        printf("\n");
        mg_mgr_poll(&mgr, 1000);

        sleep_ms(1000);
    }

    mg_mgr_free(&mgr);
    return 0;
}


// // mosquitto_pub -d -q 1 -h mqtt.thingsboard.cloud -p 1883 -t v1/devices/me/telemetry -i "Centralstation0001" -u "Centralstation0001" -P "Centralstation0001" -m "{temperature:25}"
// // To use mqtts, download the certificate here:
// // curl -f -S -o tb-cloud-root-ca.pem https://thingsboard.cloud/api/device-connectivity/mqtts/certificate/download
