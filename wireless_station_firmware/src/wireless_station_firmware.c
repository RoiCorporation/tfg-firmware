#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "nrf24_driver.h"
#include "wireless_station_firmware.h"
#include "hazards.h"
#include "alerts.h"
#include "errors.h"

void handle_nrf(nrf_client_t* nrf);

int main() {
    
    initialize_board();
    
    // Pointer to a ambient_info_t struct that stores all the data
    // read by the sensors.
    ambient_info_t sensor_readings;

    // Array of ambient_info_t elements that holds the n-previous
    // sensor readings. It's used to check for potential upcoming 
    // hazards, such as a flood, a sudden fire or a gas leak.
    ambient_info_t previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY];

    // Initialize the BH1750.
    uint8_t cmd = BH1750_CONT_H_RES_MODE;
    i2c_write_blocking(i2c0, LIGHT_SENSOR_I2C_ADDRESS, &cmd, 1, false);
    sleep_ms(180); // Wait for the first measurement (~120-180ms for high res).





    // while (1) {
    //     sensor_readings.temperature = NAN;
    //     sensor_readings.humidity = NAN;
    //     sensor_readings.pressure = NAN;
    //     sensor_readings.air_quality_index = NAN;
    //     sensor_readings.light_intensity = NAN;

    //     if (read_temperature_and_humidity(&sensor_readings) == -1) {
    //         printf("Error when reading the temperature and humidity sensor. Error number %d\n",
    //             TEMP_HUMIDITY_READ_ERROR);
    //     }
    //     else {
    //         printf("Temperature: %.1fC, Humidity: %.1f%%\n",
    //                sensor_readings.temperature, sensor_readings.humidity);
    //     }

    //     if (read_light_intensity(&sensor_readings) == -1) {
    //         printf("Error when reading the light intensity sensor. Error number %d\n",
    //             LIGHT_SENSOR_READ_ERROR);
    //     }
    //     else {
    //         printf("Light intensity: %.4f\n", sensor_readings.light_intensity);
    //     }

    //     for (int i = 0; i < LENGTH_PREVIOUS_READINGS_ARRAY - 1; i++) {
    //         previous_readings[i] = previous_readings[i + 1];
    //     }
    //     previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY - 1] = sensor_readings;

    //     int hazard_code = analyze_hazards(previous_readings);
    //     if (hazard_code != 0) {
    //         printf("Potential hazard detected! Code: %d\n", hazard_code);
    //         activate_hazard_alert(hazard_code);
    //     }

    //     
    //     // Wait another full minute before reading sensors again.
    //     sleep_ms(1000); //sleep_ms(MINUTE_IN_MILLISECONDS)
    // }


    nrf_client_t nrf24_module;
    handle_nrf(&nrf24_module);

    // payload sent to receiver data pipe 1
    uint8_t payload_one[6] = "Hello\0";

    // result of packet transmission
    fn_status_t success = 0;


    while (1) {

        // send to receiver's DATA_PIPE_1 address
        nrf24_module.tx_destination((uint8_t[]){0xC7,0xC7,0xC7,0xC7,0xC7});

        // send packet to receiver's DATA_PIPE_1 address

        success = nrf24_module.send_packet(payload_one, sizeof(payload_one));
        if (success) {
            printf("Packet sent: %s\n", payload_one);
        } 
        else {
            printf("Packet not sent:- Receiver not available.\n");
        }

        sleep_ms(500);
    }
    


}

void handle_nrf(nrf_client_t* nrf24_module) {

    pin_manager_t nrf24_pins = { .sck = SCK_PIN, .copi = COPI_PIN, .cipo = CIPO_PIN, .csn = CS_PIN, .ce = CE_PIN };

    // initialise my_nrf
    nrf_driver_create_client(nrf24_module);

    // configure GPIO pins and SPI
    nrf24_module->configure(&nrf24_pins, SPI_BAUDRATE);
    nrf24_module->dyn_payloads_enable();

    // not using default configuration (my_nrf.initialise(NULL)) 
    nrf24_module->initialise(NULL);

    // set to Standby-I Mode
    nrf24_module->standby_mode();
}