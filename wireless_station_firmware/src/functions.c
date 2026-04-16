#include <math.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "bme680_port.h"
#include "wireless_station_firmware.h"
#include "utils.h"


/**
 * @brief Initialize the different station components, such as stdio, GPIO, I2C
 * and the sensors.
 * 
 * @param bme680_sensor pointer to the struct that acts as an instance of the
 * BME680 sensor.
 * @param bme680_conf pointer to the struct for the BME680 sensor's configuration.
 * @param bme680_heater_conf pointer to the struct for the configuration of the
 * BME680 sensor's heater.
 * @param oled_display pointer to the OLED display driver.
 * @param nrf24_module pointer to the NRF24L01 module driver.
 * @param copi_pin SPI COPI microcontroller pin.
 * @param cipo_pin CIPO microcontroller pin.
 * @param sck_pin SPI SCK microcontroller pin.
 * @param cs_pin SPI CS microcontroller pin.
 * @param ce_pin SPI CE microcontroller pin.
 * @param spi_baudrate SPI baudrate (in Herz).
 */
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
) {
    stdio_init_all();
    while (!stdio_usb_connected()) sleep_ms(10);
    initialize_i2c_bus();
    sleep_ms(200);
    initialize_bme680_sensor(bme680_sensor, bme680_conf, bme680_heater_conf);
    oled_display->external_vcc=false;
    ssd1306_init(oled_display, 128, 64, OLED_DISPLAY_I2C_ADDRESS, i2c0);
    sleep_ms(200);
    ssd1306_clear(oled_display);
    initialize_nrf24_module(
        nrf24_module, copi_pin, cipo_pin, sck_pin, cs_pin, ce_pin, spi_baudrate
    );
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    unsigned int slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_set_enabled(slice_num, false);
}


/**
 * @brief Initialize the I2C bus and the pins for the SDA and SCL lines.
 */
void initialize_i2c_bus() {
    i2c_init(i2c0, I2C_BAUDRATE);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);
}


/**
 * @brief Configure the BME680 sensor.
 * 
 * @param bme680_sensor struct that acts as an instance of the sensor.
 * @param bme680_conf struct for the sensor's configuration.
 * @param bme680_heater_conf struct for the configuration of the sensor's heater.
 */
void initialize_bme680_sensor(
    struct bme68x_dev* bme680_sensor,
    struct bme68x_conf* bme680_conf,
    struct bme68x_heatr_conf* bme680_heater_conf
) {

    uint32_t del_period;
    uint32_t time_ms = 0;
    uint8_t n_fields;

    bme68x_platform_init(bme680_sensor);
    bme68x_init(bme680_sensor);

    // Set the sensor configuration and operation mode.
    bme680_conf->filter = BME68X_FILTER_OFF;
    bme680_conf->odr = BME68X_ODR_NONE;
    bme680_conf->os_hum = BME68X_OS_16X;
    bme680_conf->os_pres = BME68X_OS_1X;
    bme680_conf->os_temp = BME68X_OS_2X;
    bme68x_set_conf(bme680_conf, bme680_sensor);

    // Set the sensor heater configuration.
    bme680_heater_conf->enable = BME68X_ENABLE;
    bme680_heater_conf->heatr_temp = 300;
    bme680_heater_conf->heatr_dur = 100;
    bme68x_set_heatr_conf(BME68X_FORCED_MODE, bme680_heater_conf, bme680_sensor);
}


/**
 * @brief Configure the NRF24L01 module.
 * 
 * @param nrf24_module pointer to the nrf24l01 module driver.
 * @param copi_pin SPI COPI microcontroller pin.
 * @param cipo_pin SPI CIPO microcontroller pin.
 * @param sck_pin SPI SCK microcontroller pin.
 * @param cs_pin SPI CS microcontroller pin.
 * @param ce_pin SPI CE microcontroller pin.
 * @param spi_baudrate SPI baudrate (in Herz).
 */
void initialize_nrf24_module(
    nrf_client_t* nrf24_module,
    uint8_t copi_pin,
    uint8_t cipo_pin,
    uint8_t sck_pin,
    uint8_t cs_pin,
    uint8_t ce_pin,
    uint32_t spi_baudrate
) {
    
    pin_manager_t nrf24_pins = { 
        .copi = copi_pin,
        .cipo = cipo_pin, 
        .sck = sck_pin, 
        .csn = cs_pin, 
        .ce = ce_pin 
    };
    
    // Initialize the nrf24l01 module.
    nrf_driver_create_client(nrf24_module);
    
    // Configure GPIO pins and SPI baudrate.
    nrf24_module->configure(&nrf24_pins, spi_baudrate);
    
    // Configure the specific parameters of the module.
    nrf_manager_t nrf24_config = {
        .channel = 120,                 // RF Channel 120.
        .address_width = AW_5_BYTES,    // 5-byte address width.
        .dyn_payloads = DYNPD_ENABLE,   // Dynamic payloads enabled.
        .data_rate = RF_DR_250KBPS,     // 250 KB/s data rate.
        .power = RF_PWR_NEG_12DBM,      // -12dBm Tx output power.
        .retr_count = ARC_2RT,          // 2 packet retransmissions.
        .retr_delay = ARD_500US         // 500μS retransmission delay.
    };
    
    nrf24_module->initialise(&nrf24_config);
    // nrf24_module->initialise(NULL);
    // nrf24_module->dyn_payloads_enable();

    // Set the first data pipe with its associated address to receive any packets
    // the central station this wireless station is associated to sends it (e.g.
    // during the handshake).
    nrf24_module->rx_destination(DATA_PIPE_0, (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00});

    // Set to standby-I mode.
    nrf24_module->standby_mode();
}


/**
 * @brief Handles the handshake protocol necessary to connect this station to
 * a central station.
 * 
 * @param nrf24_module pointer to the NRF24L01 module driver.
 * @return int8_t 0 if the handshake was completed successfully, -1 otherwise.
 */
int8_t handshake(nrf_client_t *nrf24_module) {

    // Variables used in the method.
    uint8_t station_id_in_bytes[STATION_ID_BYTES_LENGTH];
    uint8_t received_packet[NRF24_ADDRESS_SIZE];

    // Convert the ID from string to an array of bytes.
    int station_id_bytes_length = hex_string_to_bytes(
        STATION_ID, station_id_in_bytes, sizeof(station_id_in_bytes));
    
    if (station_id_bytes_length != STATION_ID_BYTES_LENGTH)
        return (int8_t)-1;

    // Set the TX address to be the default handshake address that all
    // central stations listen to.
    if (nrf24_module->tx_destination((uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00}) == ERROR)
        return (int8_t)-1;

    // Transmit this station's ID as an array of bytes.
    int i = 0;
    while (i < HANDSHAKE_SEND_ID_ATTEMPTS) {
        nrf24_module->send_packet(station_id_in_bytes, sizeof(station_id_in_bytes));
        i++;
        sleep_ms(15);
    }

    /* --------------------------------------------------------------------- */
    // Now the central station will send this station the new address to which
    // it must send its readings going forward.

    // Set to module RX Mode and wait for a while to ensure it's entered RX mode.
    nrf24_module->receiver_mode();
    sleep_ms(30);

    // Read the packet.
    while (1) {
        if (nrf24_module->is_packet(NULL)) {
            if (nrf24_module->read_packet(received_packet, sizeof(received_packet)) != ERROR)
                break;
        }
        sleep_ms(30);
    }

    // Update the destination address with the new one sent by the central station.
    nrf24_module->tx_destination(received_packet);

    // Set the nrf24 module to standby-I mode to prepare it to enter TX mode and
    // wait for a while to ensure it's entered TX mode.
    nrf24_module->standby_mode();
    sleep_ms(300);

    return (int8_t)0;
}


/**
 * @brief Read temperature, humidity, air pressure and the Air Quality Index
 * (AQI) from the BME680 sensor.
 * 
 * @param bme680_sensor struct that acts as an instance of the sensor. 
 * @param bme680_conf struct for the sensor's configuration.
 * @param bme680_heater_conf struct for the configuration of the sensor's heater.
 * @param reading pointer to an ambient_info_t struct where the read values will
 * be stored.
 * @return int8_t 0 if the reading was successful, -1 otherwise.
 */
int8_t read_bme680_sensor(
    struct bme68x_dev bme680_sensor,
    struct bme68x_conf bme680_conf,
    struct bme68x_heatr_conf bme680_heater_conf,
    ambient_info_t *reading
) {
    struct bme68x_data sensor_data_read;
    uint32_t del_period;
    uint32_t time_ms = 0;
    uint8_t n_fields;
    int8_t reading_result;

    bme68x_set_op_mode(BME68X_FORCED_MODE, &bme680_sensor);
    del_period = bme68x_get_meas_dur(BME68X_FORCED_MODE, &bme680_conf, &bme680_sensor) + (bme680_heater_conf.heatr_dur * 1000);
    bme680_sensor.delay_us(del_period, bme680_sensor.intf_ptr);
    reading_result = bme68x_get_data(BME68X_FORCED_MODE, &sensor_data_read, &n_fields, &bme680_sensor);

    if (n_fields && reading_result == BME68X_OK) {
        reading->temperature = sensor_data_read.temperature;
        reading->humidity = sensor_data_read.humidity;
        reading->air_pressure = sensor_data_read.pressure;
        reading->air_quality_index = sensor_data_read.gas_resistance;
        return (int8_t)0;
    }
    return (int8_t)-1;
}


/**
 * @brief Read light intensity from the BH1750 sensor.
 * 
 * @param reading pointer to an ambient_info_t struct where the read values 
 * will be stored.
 * @return int8_t 0 if the reading was successful, -1 otherwise.
 */
int8_t read_light_intensity(ambient_info_t *reading) {
    // Try to read 2 bytes.
    uint8_t buf[2];
    int bytes_read = i2c_read_blocking(i2c0, LIGHT_SENSOR_I2C_ADDRESS, buf, 2, false);

    // If successful, calculate the actual light intensity in lux. Else, return -1.
    if (bytes_read == 2) {
        uint16_t raw = (buf[0] << 8) | buf[1];
        float lux = raw / 1.2f;
        reading->light_intensity = lux;
        return (int8_t)0;
    } 
    
    return (int8_t)-1;
}


 /**
  * @brief Transmit an encrypted radio message that contains the station's sensor
  * readings.
  * 
  * @param nrf24_module instance of the nrf24l01 driver with which the packet 
  * will be sent.
  * @param message array that contains the encrypted message to send.
  * @return int8_t 0 if the radio message was sent successfully, -1 otherwise.
  */
int8_t transmit_radio_message(nrf_client_t nrf24_module, uint8_t message[]) {
    if (nrf24_module.send_packet(message, sizeof(ambient_info_t)) != ERROR)
        return (int8_t)0;
    return (int8_t)-1;
}


/**
 * @brief Create an encrypted message from the readings of the sensors.
 * 
 * @param reading structure of sensor readings to include in the radio message.
 * @param aes_ctx AES decryption engine context.
 * @param aes_key encryption key needed to encrypt the message.
 * @param aes_iv initialization vector.
 * @param message array where the encrypted message will be stored.
 */
void encrypt_ambient_info_message(
    ambient_info_t reading,
    struct AES_ctx aes_ctx,
    const uint8_t aes_key[],
    const uint8_t aes_iv[],
    uint8_t message[]
) {

    float ambient_values_array[] = {
        reading.temperature,
        reading.humidity,
        reading.light_intensity,
        reading.air_pressure,
        reading.air_quality_index
    };

    uint8_t payload[sizeof(ambient_values_array)];
    memcpy(payload, ambient_values_array, sizeof(ambient_values_array));
    
    AES_init_ctx_iv(&aes_ctx, aes_key, aes_iv);
    AES_CTR_xcrypt_buffer(&aes_ctx, payload, sizeof(payload));

    memcpy(message, payload, sizeof(payload));
}
