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


    pin_manager_t my_pins = { .sck = SCK_PIN, .copi = COPI_PIN, .cipo = CIPO_PIN, .csn = CS_PIN, .ce = CE_PIN };



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

      /**
     * nrf_manager_t can be passed to the nrf_client_t
     * initialise function, to specify the NRF24L01 
     * configuration. If NULL is passed to the initialise 
     * function, then the default configuration will be used.
     */
    nrf_manager_t my_config = {
        // RF Channel 
        .channel = 120,

        // AW_3_BYTES, AW_4_BYTES, AW_5_BYTES
        .address_width = AW_5_BYTES,

        // dynamic payloads: DYNPD_ENABLE, DYNPD_DISABLE
        .dyn_payloads = DYNPD_ENABLE,

        // data rate: RF_DR_250KBPS, RF_DR_1MBPS, RF_DR_2MBPS
        .data_rate = RF_DR_1MBPS,

        // RF_PWR_NEG_18DBM, RF_PWR_NEG_12DBM, RF_PWR_NEG_6DBM, RF_PWR_0DBM
        .power = RF_PWR_NEG_12DBM,

        // retransmission count: ARC_NONE...ARC_15RT
        .retr_count = ARC_10RT,

        // retransmission delay: ARD_250US, ARD_500US, ARD_750US, ARD_1000US
        .retr_delay = ARD_500US 
    };

    // SPI baudrate
    uint32_t my_baudrate = 5000000;

    nrf_client_t my_nrf;

    // initialise my_nrf
    nrf_driver_create_client(&my_nrf);

    // configure GPIO pins and SPI
    my_nrf.configure(&my_pins, my_baudrate);

    // not using default configuration (my_nrf.initialise(NULL)) 
    my_nrf.initialise(&my_config);

    // set to Standby-I Mode
    my_nrf.standby_mode();

    // payload sent to receiver data pipe 0
    uint8_t payload_zero = 123;

    // payload sent to receiver data pipe 1
    uint8_t payload_one[5] = "Hello";

    typedef struct payload_two_s { uint8_t one; uint8_t two; } payload_two_t;

    // payload sent to receiver data pipe 2
    payload_two_t payload_two = { .one = 123, .two = 213 };

    // result of packet transmission
    fn_status_t success = 0;

    uint64_t time_sent = 0; // time packet was sent
    uint64_t time_reply = 0; // response time after packet sent

    while (1) {

        // send to receiver's DATA_PIPE_0 address
        my_nrf.tx_destination((uint8_t[]){0x37,0x37,0x37,0x37,0x37});

        // time packet was sent
        time_sent = to_us_since_boot(get_absolute_time()); // time sent

        // send packet to receiver's DATA_PIPE_0 address
        success = my_nrf.send_packet(&payload_zero, sizeof(payload_zero));

        // time auto-acknowledge was received
        time_reply = to_us_since_boot(get_absolute_time()); // response time

        if (success)
        {
        printf("\nPacket sent:- Response: %lluμS | Payload: %d\n", time_reply - time_sent, payload_zero);

        } else {

        printf("\nPacket not sent:- Receiver not available.\n");
        }

        sleep_ms(3000);

        // send to receiver's DATA_PIPE_1 address
        my_nrf.tx_destination((uint8_t[]){0xC7,0xC7,0xC7,0xC7,0xC7});

        // time packet was sent
        time_sent = to_us_since_boot(get_absolute_time()); // time sent

        // send packet to receiver's DATA_PIPE_1 address
        success = my_nrf.send_packet(payload_one, sizeof(payload_one));
        
        // time auto-acknowledge was received
        time_reply = to_us_since_boot(get_absolute_time()); // response time

        if (success)
        {
        printf("\nPacket sent:- Response: %lluμS | Payload: %s\n", time_reply - time_sent, payload_one);

        } else {

        printf("\nPacket not sent:- Receiver not available.\n");
        }

        sleep_ms(500);

        // send to receiver's DATA_PIPE_2 address
        my_nrf.tx_destination((uint8_t[]){0xC8,0xC7,0xC7,0xC7,0xC7});

        // time packet was sent
        time_sent = to_us_since_boot(get_absolute_time()); // time sent

        // send packet to receiver's DATA_PIPE_2 address
        success = my_nrf.send_packet(&payload_two, sizeof(payload_two));
        
        // time auto-acknowledge was received
        time_reply = to_us_since_boot(get_absolute_time()); // response time

        if (success)
        {
        printf("\nPacket sent:- Response: %lluμS | Payload: %d & %d\n",time_reply - time_sent, payload_two.one, payload_two.two);

        } else {

        printf("\nPacket not sent:- Receiver not available.\n");
        }

        sleep_ms(500);
    }
    


}