#ifndef CENTRAL_STATION_FIRMWARE_H
#define CENTRAL_STATION_FIRMWARE_H

#include <stdint.h>
#include "nrf24_driver.h"

/* ALIASES */
typedef unsigned int pin_t;

/* STRUCTS */
typedef struct
{
    float temperature;
    float humidity;
    float pressure;
    float air_quality_index;
    float light_intensity;
} ambient_info_t;

/* CONSTANTS*/
#define AMBIENT_INFO_FIELD_COUNT sizeof(ambient_info_t) / sizeof(float)
#define DHT22_PIN 0
#define BUZZER_PIN 15
#define MAX_TIMINGS 85
#define I2C_BAUDRATE 100000
#define LIGHT_SENSOR_I2C_ADDRESS 0x23
#define BH1750_CONT_H_RES_MODE 0x10
#define BOARD_ADC_RESOLUTION 4096
#define MINUTE_IN_MILLISECONDS 60000
#define LENGTH_PREVIOUS_READINGS_ARRAY 5
#define TEMPERATURE_INCREASE_MARGIN 0.2
#define HUMIDITY_INCREASE_MARGIN 2.0
#define PRESSURE_INCREASE_MARGIN 2.0
#define AIR_QUALITY_WORSENING_MARGIN 2.0
#define EPSILON 1e-5

/* I2C */
#define SDA_PIN 4
#define SCL_PIN 5

/* SPI */
#define CE_PIN 12
#define CS_PIN 17
#define SCK_PIN 18
#define COPI_PIN 19
#define CIPO_PIN 16
#define SPI_BAUDRATE 5000000

/* FUNCTION DECLARATIONS */
// Declarations for setup functions.
void initialize_board(
    nrf_client_t* nrf24_module,
    uint8_t copi_pin,
    uint8_t cipo_pin,
    uint8_t sck_pin,
    uint8_t cs_pin,
    uint8_t ce_pin,
    uint32_t spi_baudrate
);
void initialize_i2c_bus();
void initialize_nrf24_module(
    nrf_client_t* nrf24_module,
    uint8_t copi_pin,
    uint8_t cipo_pin,
    uint8_t sck_pin,
    uint8_t cs_pin,
    uint8_t ce_pin,
    uint32_t spi_baudrate
);

// Declarations for functions related to sensor readings.
int8_t read_temperature_and_humidity(ambient_info_t *reading);
int8_t read_light_intensity(ambient_info_t *reading);
int8_t receive_ambient_info(ambient_info_t *received_readings, nrf_client_t nrf24_module);


#endif
