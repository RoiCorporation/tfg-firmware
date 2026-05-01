#ifndef WIRELESS_STATION_FIRMWARE_H
#define WIRELESS_STATION_FIRMWARE_H


#include <stdint.h>
#ifndef TEST
#include "bme68x.h"
#include "ssd1306.h"
#include "nrf24_driver.h"
#include "ecdh.h"
#include "aes.h"
#endif


/* CONSTANTS */
#define AMBIENT_INFO_FIELD_COUNT sizeof(ambient_info_t) / sizeof(float)
#define STATION_ID_BYTES_LENGTH 16
#define NRF24_MAX_PACKET_SIZE 32
#define NRF24_ADDRESS_SIZE 5
#define HANDSHAKE_RETRANSMISSIONS 5
#define HANDSHAKE_MAX_READ_LOOP_ITERATIONS 100
#define TOUCH_BUTTON_PIN 7
#define BUZZER_PIN 15
#define MAX_TIMINGS 85
#define I2C_BAUDRATE 100000
#define OLED_DISPLAY_I2C_ADDRESS 0x3C
#define LIGHT_SENSOR_I2C_ADDRESS 0x23
#define BH1750_CONT_H_RES_MODE 0x10
#define BOARD_ADC_RESOLUTION 4096
#define BUTTON_PRESS_DELAY_FOR_HANDSHAKE_MS 3000
#define MINUTE_IN_MILLISECONDS 60000
#define LENGTH_PREVIOUS_READINGS_ARRAY 5
#define TEMPERATURE_INCREASE_MARGIN 1
#define TEMPERATURE_HAZARD_THRESHOLD 60.0
#define HUMIDITY_INCREASE_MARGIN 3.0
#define AIR_QUALITY_WORSENING_MARGIN 15.0
#define AIR_QUALITY_HAZARD_THRESHOLD 150.0
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

/* KDF AND AES-256 CRT */
#define KDF_SALT_SIZE 64
#define AES_KEY_SIZE 32
#define AES_IV_SIZE 16
#define AES_IV_COUNTER_SIZE 4
static uint8_t KDF_SALT[KDF_SALT_SIZE] = {
    0x77, 0xe3, 0x3e, 0x8c, 0x26, 0x0d, 0x2c, 0x61,
    0x60, 0x5c, 0x2c, 0x6c, 0xc2, 0x4a, 0xe6, 0x84, 
    0x7a, 0x2c, 0x2c, 0x72, 0x8e, 0x73, 0x20, 0x59,
    0x0c, 0xdb, 0xda, 0x76, 0xf2, 0x17, 0x75, 0xf9,
    0x63, 0x53, 0x3a, 0xde, 0x2d, 0x96, 0x10, 0x1b,
    0x94, 0x79, 0x10, 0x75, 0xe4, 0x70, 0x86, 0x3d,
    0x23, 0x9b, 0xb5, 0x98, 0xec, 0x89, 0xb0, 0x4f,
    0x23, 0x83, 0xfe, 0x1d, 0xe7, 0x3d, 0xce, 0x5b
};
static uint8_t AES_256_IV[16] = { 
    0x5a, 0x83, 0x95, 0x62, 0x3c, 0x7f, 0xa8, 0xf7,
    0x38, 0x29, 0x9b, 0x3a, 0x3e, 0x7f, 0x43, 0x5d
};

/* ENUMS */
typedef enum {
    NO_ACTION,
    TURN_ON_DISPLAY,
    START_HANDSHAKE
} button_action_t;

/* STRUCTS */
typedef struct {
    float temperature;
    float humidity;
    float light_intensity;
    float air_pressure;
    float air_quality_index;
} ambient_info_t;

#ifndef TEST
typedef struct {
    ssd1306_t *oled_display;
} button_ctx_t;

typedef struct {
    uint8_t display_turn;
    uint8_t turns_until_display_off;
    ssd1306_t *oled_display;
    ambient_info_t *station_readings;
} display_timer_ctx_t;
#endif


/* FUNCTION DECLARATIONS */
// Functions for setup and configuration.
#ifndef TEST
void initialize_station(
    struct bme68x_dev *bme680_sensor,
    struct bme68x_conf *bme680_conf,
    struct bme68x_heatr_conf *bme680_heater_conf,
    ssd1306_t *oled_display,
    nrf_client_t *nrf24_module,
    uint8_t ecdh_private_key[],
    uint8_t ecdh_public_key[],
    uint8_t copi_pin,
    uint8_t cipo_pin,
    uint8_t sck_pin,
    uint8_t cs_pin,
    uint8_t ce_pin,
    uint32_t spi_baudrate
);
void initialize_i2c_bus();
void initialize_bme680_sensor(
    struct bme68x_dev *bme680_sensor,
    struct bme68x_conf *bme680_conf,
    struct bme68x_heatr_conf *bme680_heater_conf
);
void initialize_nrf24_module(
    nrf_client_t *nrf24_module,
    uint8_t copi_pin,
    uint8_t cipo_pin,
    uint8_t sck_pin,
    uint8_t cs_pin,
    uint8_t ce_pin,
    uint32_t spi_baudrate
);
int8_t handshake(
    nrf_client_t *nrf24_module,
    uint8_t ecdh_private_key[],
    uint8_t ecdh_public_key[],
    uint8_t kdf_salt[],
    struct AES_ctx *aes_ctx,
    uint8_t aes_iv[]
);
void exit_handshake(nrf_client_t *nrf24_module);
void button_callback(uint gpio, uint32_t events);

// Functions related to sensor readings.
int8_t read_bme680_sensor(
    struct bme68x_dev bme680_sensor,
    struct bme68x_conf bme680_conf,
    struct bme68x_heatr_conf bme680_heater_conf,
    ambient_info_t *reading
);
int8_t read_temperature_and_humidity(ambient_info_t *reading);
int8_t read_light_intensity(ambient_info_t *reading);
int8_t transmit_station_readings(
    nrf_client_t *nrf24_module,
    uint8_t message[],
    size_t message_size
);

// Functions related to message encryption and decryption.
void encrypt_nrf24_payload(
    uint8_t plain_text_payload[],
    size_t plain_text_payload_size,
    uint8_t encrypted_payload[],
    struct AES_ctx *aes_ctx,
    uint8_t aes_iv[],
    uint32_t *aes_ctr_counter
);
void decrypt_nrf24_payload(
    uint8_t plain_text_payload[],
    uint8_t encrypted_payload[],
    size_t encrypted_payload_size,
    struct AES_ctx *aes_ctx,
    uint8_t aes_iv[]
);
void encrypt_ambient_info_message(
    ambient_info_t *reading,
    struct AES_ctx *aes_ctx,
    uint8_t aes_iv[],
    uint8_t message[]
);
#endif


#endif
