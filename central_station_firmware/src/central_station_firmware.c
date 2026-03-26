#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "central_station_firmware.h"
#include "errors.h"


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

            printf("Temperature: %f\n", wireless_station_info.temperature);
            printf("Humidity: %f\n", wireless_station_info.humidity);
            printf("Light intensity: %f\n", wireless_station_info.light_intensity);
            printf("Pressure: %f\n", wireless_station_info.air_pressure);
            printf("Air quality index: %f\n", wireless_station_info.air_quality_index);
        }
        printf("\n");

        sleep_ms(1000);
    }
}
