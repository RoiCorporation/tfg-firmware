#ifndef WIRELESS_STATION_FIRMWARE_H
#define WIRELESS_STATION_FIRMWARE_H

#include <stdint.h>
#ifndef TEST
#include "bme68x.h"
#include "ssd1306.h"
#include "nrf24_driver.h"
#include "aes.h"
#endif


/* CONSTANTS*/
#define AMBIENT_INFO_FIELD_COUNT sizeof(ambient_info_t) / sizeof(float)
#define STATION_ID_BYTES_LENGTH 16
#define NRF24_ADDRESS_SIZE 5
#define HANDSHAKE_SEND_ID_ATTEMPTS 10
#define DHT22_PIN 0
#define BUZZER_PIN 15
#define MAX_TIMINGS 85
#define I2C_BAUDRATE 100000
#define OLED_DISPLAY_I2C_ADDRESS 0x3C
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

/* AES-256 CRT */
static const uint8_t AES_256_KEY[32] = {
    0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
    0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4 
};
static const uint8_t AES_256_IV[16] = { 
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff 
};

/* STRUCTS */
typedef struct {
    float temperature;
    float humidity;
    float light_intensity;
    float air_pressure;
    float air_quality_index;
} ambient_info_t;


/* FUNCTION DECLARATIONS */
// Declarations for setup functions.
#ifndef TEST
void initialize_station(
    struct bme68x_dev* bme680_sensor,
    struct bme68x_conf* bme680_conf,
    struct bme68x_heatr_conf* bme680_heater_conf,
    ssd1306_t *oled_display,
    nrf_client_t* nrf24_module,
    uint8_t copi_pin,
    uint8_t cipo_pin,
    uint8_t sck_pin,
    uint8_t cs_pin,
    uint8_t ce_pin,
    uint32_t spi_baudrate
);
void initialize_i2c_bus();
void initialize_bme680_sensor(
    struct bme68x_dev* bme680_sensor,
    struct bme68x_conf* bme680_conf,
    struct bme68x_heatr_conf* bme680_heater_conf
);
void initialize_nrf24_module(
    nrf_client_t* nrf24_module,
    uint8_t copi_pin,
    uint8_t cipo_pin,
    uint8_t sck_pin,
    uint8_t cs_pin,
    uint8_t ce_pin,
    uint32_t spi_baudrate
);
int8_t handshake(nrf_client_t *nrf24_module);

// Declarations for functions related to sensor readings.
int8_t read_bme680_sensor(
    struct bme68x_dev bme680_sensor,
    struct bme68x_conf bme680_conf,
    struct bme68x_heatr_conf bme680_heater_conf,
    ambient_info_t *reading
);
int8_t read_temperature_and_humidity(ambient_info_t *reading);
int8_t read_light_intensity(ambient_info_t *reading);
int8_t transmit_radio_message(nrf_client_t nrf24_module, uint8_t message[]);
void encrypt_ambient_info_message(
    ambient_info_t reading,
    struct AES_ctx aes_ctx,
    const uint8_t aes_key[],
    const uint8_t aes_iv[],
    uint8_t message[]
);
#endif


#endif
