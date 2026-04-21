#ifndef CENTRAL_STATION_FIRMWARE_H
#define CENTRAL_STATION_FIRMWARE_H

#include <stdint.h>
#ifndef TEST
#include "bme68x.h"
#include "ssd1306.h"
#include "nrf24_driver.h"
#include "aes.h"
#include "mongoose.h"
#endif


/* CONSTANTS*/
#define AMBIENT_INFO_FIELD_COUNT (sizeof(ambient_info_t) - STATION_ID_CHAR_LENGTH) / sizeof(float)
#define STATION_ID_BYTES_LENGTH 16
#define STATION_ID_CHAR_LENGTH 37
#define NRF24_ADDRESS_SIZE 5
#define NRF24_ADDRESSES_BUFFER_SIZE 6
#define TOUCH_BUTTON_PIN 7
#define BUZZER_PIN 15
#define MAX_TIMINGS 85
#define I2C_BAUDRATE 400000
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
#define CARBON_MONOXIDE_WORSENING_MARGIN 10.0
#define CARBON_MONOXIDE_HAZARD_THRESHOLD 70.0
#define METHANE_WORSENING_MARGIN 10.0
#define METHANE_HAZARD_THRESHOLD 1000.0
#define PROPANE_WORSENING_MARGIN 10.0
#define PROPANE_HAZARD_THRESHOLD 1100.0
#define ALCOHOL_WORSENING_MARGIN 0.2
#define ALCOHOL_HAZARD_THRESHOLD 2.0
#define HYDROGEN_GAS_WORSENING_MARGIN 10.0
#define HYDROGEN_GAS_HAZARD_THRESHOLD 4000.0
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

/* ENUMS */
typedef enum {
    NO_ACTION,
    TURN_ON_DISPLAY,
    START_HANDSHAKE
} button_action_t;

/* STRUCTS */
typedef struct {
    char station_id[STATION_ID_CHAR_LENGTH];
    float temperature;
    float humidity;
    float light_intensity;
    float air_pressure;
    float air_quality_index;
    float carbon_monoxide_concentration;
    float methane_concentration;
    float propane_concentration;
    float alcohol_concentration;
    float hydrogen_gas_concentration;
} ambient_info_t;

typedef struct {
    struct mg_mgr *connection_manager;
    struct mg_connection *mqtt_connection;
    int8_t is_mqtt_connection_ready;
    ambient_info_t environmental_readings;
} network_ctx_t;

typedef struct {
    uint8_t nrf24l01_address[NRF24_ADDRESS_SIZE];
    char *associated_station_id;
} station_id_address_map_t;

#ifndef TEST
typedef struct {
    struct bme68x_dev bme680_sensor;
    struct bme68x_conf bme680_conf;
    struct bme68x_heatr_conf bme680_heater_conf;
    ssd1306_t *oled_display;
    network_ctx_t network_context;
} queue_entry_t;
#endif


/* FUNCTION DECLARATIONS */
// Declarations for setup functions.
#ifndef TEST
void initialize_station(
    struct bme68x_dev *bme680_sensor,
    struct bme68x_conf *bme680_conf,
    struct bme68x_heatr_conf *bme680_heater_conf,
    ssd1306_t *oled_display,
    nrf_client_t *nrf24_module,
    station_id_address_map_t station_id_to_nrf24_address_buffer[],
    struct mg_mgr *connection_manager,
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
    station_id_address_map_t station_id_to_nrf24_address_buffer[],
    uint8_t copi_pin,
    uint8_t cipo_pin,
    uint8_t sck_pin,
    uint8_t cs_pin,
    uint8_t ce_pin,
    uint32_t spi_baudrate
);
int8_t handshake(
    nrf_client_t *nrf24_module,
    station_id_address_map_t station_id_to_nrf24_address_buffer[],
    size_t buffer_size
);
void button_callback(uint gpio, uint32_t events);

// Declarations for functions related to sensor readings.
int8_t read_bme680_sensor(
    struct bme68x_dev bme680_sensor,
    struct bme68x_conf bme680_conf,
    struct bme68x_heatr_conf bme680_heater_conf,
    ambient_info_t *reading
);
int8_t read_temperature_and_humidity(ambient_info_t *reading);
int8_t read_light_intensity(ambient_info_t *reading);
int8_t receive_radio_message(
    nrf_client_t nrf24_module,
    uint8_t message[],
    uint8_t *incoming_packet_data_pipe
);
void decrypt_ambient_info_message(
    ambient_info_t *received_readings, 
    struct AES_ctx aes_ctx, 
    const uint8_t aes_key[], 
    const uint8_t aes_iv[], 
    uint8_t message[]
);
#endif


#endif
