#include "stdio.h"
#include <math.h>
#include "pico/stdlib.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "nrf24_driver.h"
#include "central_station_firmware.h"


int main() {

    // Instance of the nrf24l01 module driver.
    nrf_client_t nrf24_module;
    
    ambient_info_t wireless_station_info;

    // Configure all the protocols and pins in the board.
    initialize_board(&nrf24_module, COPI_PIN, CIPO_PIN, SCK_PIN, CS_PIN, CE_PIN, SPI_BAUDRATE);

    while (!stdio_usb_connected()) sleep_ms(10);
    
    while (1) {
        wireless_station_info.temperature = NAN;
        wireless_station_info.humidity = NAN;
        wireless_station_info.light_intensity = NAN;
        wireless_station_info.air_pressure = NAN;
        wireless_station_info.air_quality_index = NAN;

        receive_ambient_info(&wireless_station_info, nrf24_module);

        printf("Temperature: %f\n", wireless_station_info.temperature);
        printf("Humidity: %f\n", wireless_station_info.humidity);
        printf("Light intensity: %f\n", wireless_station_info.light_intensity);
        printf("Pressure: %f\n", wireless_station_info.air_pressure);
        printf("Air quality index: %f\n", wireless_station_info.air_quality_index);

        sleep_ms(1000);
    }

}
