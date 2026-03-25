#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "bme68x.h"
#include "bme680_port.h"
#include "nrf24_driver.h"
#include "aes.h"
#include "wireless_station_firmware.h"
#include "hazards.h"
#include "alerts.h"
#include "errors.h"


int main() {

    // Pointer to a ambient_info_t struct that stores all the data
    // read by the sensors.
    ambient_info_t sensor_readings;
    
    // Array of ambient_info_t elements that holds the n-previous
    // sensor readings. It's used to check for potential upcoming 
    // hazards, such as a flood, a sudden fire or a gas leak.
    ambient_info_t previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY];
    
    // NRF24L01 module driver.
    nrf_client_t nrf24_module;

    // AES encryption engine context.
    struct AES_ctx aes_ctx;

    uint8_t radio_message[sizeof(ambient_info_t)];

    // Configure all the protocols and pins in the board.
    initialize_board(&nrf24_module, COPI_PIN, CIPO_PIN, SCK_PIN, CS_PIN, CE_PIN, SPI_BAUDRATE);

    while (!stdio_usb_connected()) sleep_ms(10);

    // Initialize the BH1750.
    uint8_t cmd = BH1750_CONT_H_RES_MODE;
    i2c_write_blocking(i2c0, LIGHT_SENSOR_I2C_ADDRESS, &cmd, 1, false);
    sleep_ms(180); // Wait for the first measurement (~120-180ms for high res).




    int8_t rslt;
    struct bme68x_conf conf;
    struct bme68x_heatr_conf heatr_conf;
    struct bme68x_data data;
    uint32_t del_period;
    uint32_t time_ms = 0;
    uint8_t n_fields;
    uint16_t sample_count = 20;

    /* Interface preference is updated as a parameter
    * For I2C : BME68X_I2C_INTF
    * For SPI : BME68X_SPI_INTF
    */
    struct bme68x_dev bme680 = {0};
    rslt = bme68x_platform_init(&bme680);
    if (rslt != BME68X_OK) {
        printf("platform init failed: %d\n", rslt);
        while (1) sleep_ms(1000);
    }

    rslt = bme68x_init(&bme680);
    if (rslt != BME68X_OK) {
        printf("bme68x_init failed: %d\n", rslt);
        while (1) sleep_ms(1000);
    }

    printf("BME68x init OK\n");

    /* Check if rslt == BME68X_OK, report or handle if otherwise */
    conf.filter = BME68X_FILTER_OFF;
    conf.odr = BME68X_ODR_NONE;
    conf.os_hum = BME68X_OS_16X;
    conf.os_pres = BME68X_OS_1X;
    conf.os_temp = BME68X_OS_2X;
    rslt = bme68x_set_conf(&conf, &bme680);

    /* Check if rslt == BME68X_OK, report or handle if otherwise */
    heatr_conf.enable = BME68X_ENABLE;
    heatr_conf.heatr_temp = 300;
    heatr_conf.heatr_dur = 100;
    rslt = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &bme680);


    printf("Temperature(deg C), Pressure(Pa), Humidity(%%), Gas resistance(ohm), Status\n");


    while (1) {
        sensor_readings.temperature = NAN;
        sensor_readings.humidity = NAN;
        sensor_readings.light_intensity = NAN;
        sensor_readings.air_pressure = NAN;
        sensor_readings.air_quality_index = NAN;

        if (read_temperature_and_humidity(&sensor_readings) == -1) {
            printf("Error when reading the temperature and humidity sensor. Error number %d\n",
                TEMP_HUMIDITY_READ_ERROR);
        }
        else {
            printf("DHT22 readings: Temperature %.1fC, Humidity %.1f%%\n",
                   sensor_readings.temperature, sensor_readings.humidity);
        }

        // if (read_light_intensity(&sensor_readings) == -1) {
        //     printf("Error when reading the light intensity sensor. Error number %d\n",
        //         LIGHT_SENSOR_READ_ERROR);
        // }
        // else {
        //     printf("Light intensity: %.4f\n", sensor_readings.light_intensity);
        // }

        // for (int i = 0; i < LENGTH_PREVIOUS_READINGS_ARRAY - 1; i++) {
        //     previous_readings[i] = previous_readings[i + 1];
        // }
        // previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY - 1] = sensor_readings;

        // int hazard_code = analyze_hazards(previous_readings);
        // if (hazard_code != 0) {
        //     printf("Potential hazard detected! Code: %d\n", hazard_code);
        //     activate_hazard_alert(hazard_code);
        // }

        // encrypt_ambient_info_message(
        //     sensor_readings, aes_ctx, AES_256_KEY, AES_256_IV, radio_message);

        // if (transmit_radio_message(nrf24_module, radio_message) == -1) {
        //     printf("Error when sending the sensor readings. Error number %d\n", 
        //         DATA_TRANSMIT_ERROR);
        // }
        // else {
        //     printf("Packet transmitted successfully.\n");
        // }
        // printf("\n");


        rslt = bme68x_set_op_mode(BME68X_FORCED_MODE, &bme680);


        /* Calculate delay period in microseconds */
        del_period = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &bme680) + (heatr_conf.heatr_dur * 1000);
        bme680.delay_us(del_period, bme680.intf_ptr);

        /* Check if rslt == BME68X_OK, report or handle if otherwise */
        rslt = bme68x_get_data(BME68X_FORCED_MODE, &data, &n_fields, &bme680);

        if (n_fields)
        {
            printf("BME680 readings: Temperature %.2f, Humidity %.2f, Air pressure (Pa) %.2f, Gas resistance (ohm) %d, Status 0x%x\n",
                data.temperature, data.humidity, data.pressure, data.gas_resistance, data.status);
            sample_count++;
        }

        else if (rslt != BME68X_OK){
            printf("Error reading\n");
        }

        printf("\n");
        // Wait another full minute before reading sensors again.
        sleep_ms(1000); //sleep_ms(MINUTE_IN_MILLISECONDS)
    }

}
