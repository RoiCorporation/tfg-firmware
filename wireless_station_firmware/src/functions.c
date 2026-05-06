#include <math.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "pico/rand.h"
#include "pico/cyw43_arch.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "wireless_station_firmware.h"
#include "callbacks.h"
#include "bme680_port.h"
#include "utils.h"
#include "ecdh.h"
#include "low_power.h"


#include <stdio.h>

extern retained_data_t data_retained_in_hibernation;
extern volatile button_action_t button_action;


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
 * @param ecdh_private_key uint8_t array that will store the station's private key
 * to generate an ECDH public key.
 * @param ecdh_public_key uint8_t array that will store the station's ECDH public
 * key to send in a handshake association attempt with another station.
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
    uint8_t ecdh_private_key[],
    uint8_t ecdh_public_key[],
    uint8_t copi_pin,
    uint8_t cipo_pin,
    uint8_t sck_pin,
    uint8_t cs_pin,
    uint8_t ce_pin,
    uint32_t spi_baudrate
) {

    // Initialize the global variables.
    button_action = TURN_ON_DISPLAY;

    // Configure every component, protocol, GPIO,...
    stdio_init_all();
    initialize_i2c_bus();
    sleep_ms(200);
    cyw43_arch_deinit(); // Not used by this type of station.

    initialize_bme680_sensor(bme680_sensor, bme680_conf, bme680_heater_conf);
    oled_display->external_vcc=false;
    ssd1306_init(oled_display, 128, 64, OLED_DISPLAY_I2C_ADDRESS, i2c0);
    sleep_ms(200);
    ssd1306_clear(oled_display);
    initialize_nrf24_module(
        nrf24_module, copi_pin, cipo_pin, sck_pin, cs_pin, ce_pin, spi_baudrate
    );

    // Generate an ECDH private key using random bytes. Then, use
    // that private key to generate an ECDH public key.
    for (int i = 0; i < ECC_PRV_KEY_SIZE; i++)
        ecdh_private_key[i] = (uint8_t)get_rand_32();
    ecdh_generate_keys(ecdh_public_key, ecdh_private_key);

    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    unsigned int slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_set_enabled(slice_num, false);

    // Initialize the BH1750.
    uint8_t cmd = BH1750_CONT_H_RES_MODE;
    i2c_write_blocking(i2c0, LIGHT_SENSOR_I2C_ADDRESS, &cmd, 1, false);

    // Initialize the touch button.
    gpio_init(TOUCH_BUTTON_PIN);
    gpio_set_irq_enabled_with_callback(
        TOUCH_BUTTON_PIN,
        GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
        true,
        &button_callback
    );
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
 * @param bme680_sensor pointer to the struct that acts as an instance of the sensor.
 * @param bme680_conf pointer to the struct for the sensor's configuration.
 * @param bme680_heater_conf pointer to the struct that holds the configuration
 * for the sensor's heater.
 */
void initialize_bme680_sensor(
    struct bme68x_dev *bme680_sensor,
    struct bme68x_conf *bme680_conf,
    struct bme68x_heatr_conf *bme680_heater_conf
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
        .retr_count = ARC_15RT,          // 2 packet retransmissions.
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
 * @brief Implements the handshake protocol necessary to associate this station
 * with a central one.
 * 
 * @param nrf24_module pointer to the NRF24L01 module driver.
 * @param ecdh_private_key uint8_t array that stores the station's ECDH private
 * key.
 * @param ecdh_public_key uint8_t array that stores the station's ECDH public
 * key that gets sent to the other station at the beginning of the handshake.
 * @param kdf_salt uint8_t array containing the salt used in the KDF.
 * @param aes_ctx pointer to the AES encryption module.
 * @param aes_key uint8_t array that will store the AES encryption key after
 * invoking the KDF.
 * @param aes_iv uint8_t array containing the initialization vector with which
 * to set up the AES encryption module.
 * @return int8_t 0 if the handshake was completed successfully, -1 otherwise.
 */
int8_t handshake(
    nrf_client_t *nrf24_module,
    uint8_t ecdh_private_key[],
    uint8_t ecdh_public_key[],
    uint8_t kdf_salt[],
    struct AES_ctx *aes_ctx,
    uint8_t aes_iv[]
) {

    printf("ENTERED HANDSHAKE\n");

    // Variables used in the method.
    int read_loop_iterations = 0;
    uint8_t other_station_ecdh_public_key[ECC_PUB_KEY_SIZE] = {0x00};
    uint8_t aux_pub_key_first_half_buffer[ECC_PUB_KEY_SIZE / 2] = {0x00};
    uint8_t aux_pub_key_second_half_buffer[ECC_PUB_KEY_SIZE / 2] = {0x00};
    uint8_t shared_secret[ECC_PUB_KEY_SIZE] = {0x00};
    uint8_t aes_key[AES_KEY_SIZE] = {0x00};
    uint8_t station_id_in_bytes[STATION_ID_BYTES_LENGTH] = {0x00};
    uint8_t station_id_packet[STATION_ID_BYTES_LENGTH + AES_IV_COUNTER_SIZE] = {0x00};
    uint8_t nrf24_address_packet[NRF24_ADDRESS_SIZE + AES_IV_COUNTER_SIZE] = {0x00};
    uint8_t final_nrf24_address[NRF24_ADDRESS_SIZE] = {0x00};
    uint8_t aux_flush_rx_fifo_buffer[NRF24_MAX_PACKET_SIZE] = {0x00};
    uint8_t handshake_result_packet[NRF24_MAX_PACKET_SIZE] = {0x00};
    uint8_t handshake_result[NRF24_MAX_PACKET_SIZE - AES_IV_COUNTER_SIZE] = {0x00};


    // Set the TX address to be the default handshake address that all
    // central stations listen to.
    if (nrf24_module->tx_destination(
        (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00}) == ERROR)
        return (int8_t)-1;

    // Time buffer to ensure the central station has had time to get ready
    // for reading the packets.
    sleep_ms(2000);
  
    /* --------------------------------------------------------------------- */
    // Send the first half of this station's ECDH public key.

    // Divide the public key in two halves.
    memcpy(aux_pub_key_first_half_buffer, ecdh_public_key, ECC_PUB_KEY_SIZE / 2);
    memcpy(aux_pub_key_second_half_buffer, ecdh_public_key + ECC_PUB_KEY_SIZE / 2, ECC_PUB_KEY_SIZE / 2);

    printf("REACHED 1\n");
    // Send the packet with the first half of the station's ECDH public key.
    for (int i = 0; i < HANDSHAKE_RETRANSMISSIONS; i++) {
        nrf24_module->send_packet(ecdh_public_key, ECC_PUB_KEY_SIZE / 2);
        sleep_ms(5);
    }
    printf("REACHED 1.5\n");

    /* --------------------------------------------------------------------- */
    // Receive the first half of the central station's ECDH public key.

    // Set to module RX mode and wait for a while to ensure it's entered RX mode.
    nrf24_module->receiver_mode();
    sleep_ms(20);

    printf("REACHED 2\n");
    // Read the packet.
    while (1) {
        if (nrf24_module->is_packet(NULL)) {

            // If a packet is successfully read, save that packet as first half
            // of the central station's ECDH public key.
            if (nrf24_module->read_packet(
                aux_pub_key_first_half_buffer,
                sizeof(aux_pub_key_first_half_buffer)
            ) != ERROR)
                break;
        }
        read_loop_iterations++;
        if (read_loop_iterations >= HANDSHAKE_MAX_READ_LOOP_ITERATIONS) {
            printf("FAIL 2\n");
            exit_handshake(nrf24_module);
            return (int8_t)-1;
        }

        sleep_ms(10);
    }
    read_loop_iterations = 0;

    /* --------------------------------------------------------------------- */
    // Send the second half of this station's ECDH public key.

    // Set the nrf24 module to standby-I mode to prepare it to enter TX mode and
    // wait for a while to ensure it's entered TX mode.
    nrf24_module->standby_mode();
    sleep_ms(30);

    printf("REACHED 3\n");
    // Send the packet with the second half of the station's ECDH public key.
    for (int i = 0; i < HANDSHAKE_RETRANSMISSIONS; i++) {
        nrf24_module->send_packet(ecdh_public_key + ECC_PUB_KEY_SIZE / 2, ECC_PUB_KEY_SIZE / 2);
        sleep_ms(5);
    }

    /* --------------------------------------------------------------------- */
    // Receive the second half of the central station's ECDH public key.

    // Set to module RX mode and wait for a while to ensure it's entered RX mode.
    nrf24_module->receiver_mode();
    sleep_ms(20);

    printf("REACHED 4\n");
    // Read the packet.
    while (1) {
        if (nrf24_module->is_packet(NULL)) {

            // If a packet is successfully read, save that packet as second half
            // of the other station's ECDH public key.
            if (nrf24_module->read_packet(
                aux_pub_key_second_half_buffer,
                sizeof(aux_pub_key_second_half_buffer)
            ) != ERROR)
                break;
        }
        read_loop_iterations++;
        if (read_loop_iterations >= HANDSHAKE_MAX_READ_LOOP_ITERATIONS) {
            exit_handshake(nrf24_module);
            printf("FAIL 4\n");
            return (int8_t)-1;
        }

        sleep_ms(10);
    }
    read_loop_iterations = 0;

    /* --------------------------------------------------------------------- */
    // Obtain the AES key.

    // Add both halves together and generate the shared secret.
    memcpy(other_station_ecdh_public_key, aux_pub_key_first_half_buffer, ECC_PUB_KEY_SIZE / 2);
    memcpy(other_station_ecdh_public_key + ECC_PUB_KEY_SIZE / 2, aux_pub_key_second_half_buffer, ECC_PUB_KEY_SIZE / 2);

    // Calculate the shared secret using the station's ECDH private key and the
    // public key from the other station.
    if (ecdh_shared_secret(
        ecdh_private_key,
        other_station_ecdh_public_key,
        shared_secret
    ) == 0)
        return (int8_t)-1;

    // Invoke the KDF procedure to generate the AES encryption key that will be
    // used from here on to encrypt and/or decrypt radio packets.
    kdf(
        aes_key,
        AES_KEY_SIZE,
        shared_secret,
        sizeof(shared_secret),
        kdf_salt,
        KDF_SALT_SIZE
    );

    // Initialize the AES encryption module.
    AES_init_ctx_iv(aes_ctx, aes_key, aes_iv);

    /* --------------------------------------------------------------------- */
    // Send the station's ID encrypted by using the AES encryption module.

    // Set the nrf24 module to standby-I mode to prepare it to enter TX mode and
    // wait for a while to ensure it's entered TX mode.
    nrf24_module->standby_mode();
    sleep_ms(300);

    // Convert the ID from string to an array of bytes.
    int station_id_bytes_length = hex_string_to_bytes(
        STATION_ID, station_id_in_bytes, sizeof(station_id_in_bytes));
    
    if (station_id_bytes_length != STATION_ID_BYTES_LENGTH)
        return (int8_t)-1;

    // Encrypt the message using the AES encryption module set up above.
    encrypt_nrf24_payload(
        station_id_in_bytes,
        sizeof(station_id_in_bytes),
        station_id_packet,
        aes_ctx,
        aes_iv,
        &data_retained_in_hibernation.aes_ctr_counter
    );

    // Transmit this station's ID as an array of bytes.
    for (int i = 0; i < HANDSHAKE_RETRANSMISSIONS; i++) {
        nrf24_module->send_packet(station_id_packet, sizeof(station_id_packet));
        sleep_ms(15);
    }

    /* --------------------------------------------------------------------- */
    // Receive the new NRF24L01 address to which this station must send its
    // readings going forward.

    // Set to module RX mode and wait for a while to ensure it's entered RX mode.
    nrf24_module->receiver_mode();
    sleep_ms(30);

    // Flush the NRF24L01 RX FIFO to prevent the NRF24 module from rejecting
    // inbound messages.
    nrf24_module->read_packet(
        aux_flush_rx_fifo_buffer,
        sizeof(aux_flush_rx_fifo_buffer)
    );

    // Read the packet.
    while (1) {
        if (nrf24_module->is_packet(NULL)) {
            if (nrf24_module->read_packet(
                nrf24_address_packet,
                sizeof(nrf24_address_packet)
            ) != ERROR)
                break;
        }
        read_loop_iterations++;
        if (read_loop_iterations >= HANDSHAKE_MAX_READ_LOOP_ITERATIONS) {
            exit_handshake(nrf24_module);
            return (int8_t)-1;
        }

        sleep_ms(20);
    }
    read_loop_iterations = 0;

    // Decrypt the message containing the final NRF24 destination address
    // using the AES encryption module set up above.
    decrypt_nrf24_payload(
        final_nrf24_address,
        nrf24_address_packet,
        sizeof(nrf24_address_packet),
        aes_ctx,
        aes_iv
    );

    // Update the destination address with the new one sent by the central station.
    nrf24_module->tx_destination(final_nrf24_address);

    /* --------------------------------------------------------------------- */
    // Send the received NRF24 address to check it was received correctly.

    // Set the nrf24 module to standby-I mode to prepare it to enter TX mode and
    // wait for a while to ensure it's entered TX mode.
    nrf24_module->standby_mode();
    sleep_ms(300);

    // Encrypt the message using the AES encryption module set up above.
    encrypt_nrf24_payload(
        final_nrf24_address,
        sizeof(final_nrf24_address),
        nrf24_address_packet,
        aes_ctx,
        aes_iv,
        &data_retained_in_hibernation.aes_ctr_counter
    );

    // Transmit the NRF24 address to check that it was received correctly.
    for (int i = 0; i < HANDSHAKE_RETRANSMISSIONS; i++) {
        nrf24_module->send_packet(
            nrf24_address_packet,
            sizeof(nrf24_address_packet)
        );
        sleep_ms(15);
    }

    /* --------------------------------------------------------------------- */
    // Receive the result of comparing the NRF24 address received and the
    // original address sent by the wireless station.

    // Set to module RX mode and wait for a while to ensure it's entered RX mode.
    nrf24_module->receiver_mode();
    sleep_ms(30);

    // Read the packet.
    while (1) {
        if (nrf24_module->is_packet(NULL)) {
            if (nrf24_module->read_packet(
                handshake_result_packet,
                sizeof(handshake_result_packet)
            ) != ERROR)
                break;
        }
        read_loop_iterations++;
        if (read_loop_iterations >= HANDSHAKE_MAX_READ_LOOP_ITERATIONS) {
            exit_handshake(nrf24_module);
            return (int8_t)-1;
        }

        sleep_ms(20);
    }
    read_loop_iterations = 0;

    // Decrypt the message containing the result of the comparison
    // using the AES encryption module set up above.
    decrypt_nrf24_payload(
        handshake_result,
        handshake_result_packet,
        sizeof(handshake_result_packet),
        aes_ctx,
        aes_iv
    );

    // Set the nrf24 module to standby-I mode to prepare it to enter TX mode and
    // wait for a while to ensure it's entered TX mode.
    nrf24_module->standby_mode();
    sleep_ms(300);

    // If the address comparison wasn't successful, exit the handshake returning
    // an error. Otherwise, the handshake was completed successfully.
    for (int i = 0; i < sizeof(handshake_result); i++) {
        if (handshake_result[i] != 0x00)
            return (int8_t)-1;
    }
    printf("ALL GOOD\n");
    return (int8_t)0;
}


/**
 * @brief Handle the exit of the handshake protocol on failure, resetting the
 * configuration of the NRF24L01 module to its original setup.
 * 
 * @param nrf24_module pointer to the NRF24L01 module driver.
 */
void exit_handshake(nrf_client_t *nrf24_module) {

    // Reset the AES CTR counter.
    data_retained_in_hibernation.aes_ctr_counter = 0;

    // Set the TX address to be the default handshake address that all
    // central stations listen to.
    nrf24_module->tx_destination((uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00});

    // Set to standby-I mode.
    nrf24_module->standby_mode();
    sleep_ms(300);
}


void hibernate() {
    printf("GOING INTO LOW POWER\n");
    turn_low_power_on();
}


/**
 * @brief Read temperature, humidity, air pressure and the Air Quality Index
 * (AQI) from the BME680 sensor.
 * 
 * @param bme680_sensor pointer to the struct that acts as an instance of the
 * sensor. 
 * @param bme680_conf pointer to the struct that holds the sensor's configuration.
 * @param bme680_heater_conf pointer to the struct that holds the configuration
 * of the sensor's heater.
 * @param reading pointer to an ambient_info_t struct where the read values will
 * be stored.
 * @return int8_t 0 if the reading was successful, -1 otherwise.
 */
int8_t read_bme680_sensor(
    struct bme68x_dev *bme680_sensor,
    struct bme68x_conf *bme680_conf,
    struct bme68x_heatr_conf *bme680_heater_conf,
    ambient_info_t *reading
) {
    struct bme68x_data sensor_data_read;
    uint32_t del_period;
    uint32_t time_ms = 0;
    uint8_t n_fields;
    int8_t reading_result;

    bme68x_set_op_mode(BME68X_FORCED_MODE, bme680_sensor);
    del_period = bme68x_get_meas_dur(
        BME68X_FORCED_MODE,
        bme680_conf, bme680_sensor) + (bme680_heater_conf->heatr_dur * 1000
    );
    bme680_sensor->delay_us(del_period, bme680_sensor->intf_ptr);
    reading_result = bme68x_get_data(BME68X_FORCED_MODE, &sensor_data_read, &n_fields, bme680_sensor);

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
  * @param nrf24_module pointer to the NRF24L01 module driver.
  * @param message uint8_t array that contains the encrypted message to send.
  * @param message_size size_t size of the encrypted message buffer.
  * @return int8_t 0 if the radio message was sent successfully, -1 otherwise.
  */
int8_t transmit_station_readings(
    nrf_client_t *nrf24_module,
    uint8_t message[],
    size_t message_size
) {

    if (nrf24_module->send_packet(message, message_size) != ERROR)
        return (int8_t)0;
    return (int8_t)-1;
}


/**
 * @brief Encrypt the given plain text NRF24L01 packet payload using the AES
 * encryption module configured in the handshake.
 * 
 * @param plain_text_payload uint8_t array containing the plain text payload to
 * encrypt.
 * @param plain_text_payload_size size_t size of the plain text payload buffer.
 * @param encrypted_payload uint8_t array that will contain the encrypted payload,
 * made up of the encrypted plain text payload plus four plain text AES CTR
 * counter bytes.
 * @param aes_ctx pointer to the AES encryption module.
 * @param aes_iv uint8_t array containing the initialization vector for the AES
 * CTR encryption/decryption scheme.
 * @param aes_ctr_counter uint32_t value used as the AES CTR counter.
 */
void encrypt_nrf24_payload(
    uint8_t plain_text_payload[],
    size_t plain_text_payload_size,
    uint8_t encrypted_payload[],
    struct AES_ctx *aes_ctx,
    uint8_t aes_iv[],
    uint32_t *aes_ctr_counter
) {

    // Convert the AES CTR counter number to an array of 4 bytes.
    uint8_t aes_ctr_counter_array[AES_IV_COUNTER_SIZE];
    uint32_to_u8_be(*aes_ctr_counter, aes_ctr_counter_array);

    // Replace the last 4 bytes of the AES initialization vector with the array
    // created above.
    memcpy(
        aes_iv + AES_IV_SIZE - 4,
        aes_ctr_counter_array,
        sizeof(aes_ctr_counter_array)
    );

    // Copy the plain text payload into the encrypted payload buffer.
    memcpy(
        encrypted_payload,
        plain_text_payload,
        plain_text_payload_size
    );

    // Update the AES encryption module initialization vector with its new
    // version and encrypt the section of the payload corresponding to the
    // plain text payload.
    AES_ctx_set_iv(aes_ctx, aes_iv);
    AES_CTR_xcrypt_buffer(aes_ctx, encrypted_payload, plain_text_payload_size);

    // Copy the array containing the bytes of the numeric AES CTR counter
    // right at the end of the encrypted payload buffer.
    memcpy(
        encrypted_payload + plain_text_payload_size,
        aes_ctr_counter_array,
        AES_IV_COUNTER_SIZE
    );

    // Update the AES CTR counter.
    (*aes_ctr_counter)++;
}


/**
 * @brief Decrypt the encrypted NRF24L01 packet payload using the AES encryption
 * module configured in the handshake.
 * 
 * @param plain_text_payload uint8_t array that will contain the decrypted
 * payload.
 * @param encrypted_payload uint8_t array containing the encrypted payload,
 * made up of the encrypted plain text payload plus four plain text AES CTR
 * counter bytes.
 * @param encrypted_payload_size size_t size of the encrypted payload buffer.
 * @param aes_ctx pointer to the AES encryption module.
 * @param aes_iv uint8_t array containing the initialization vector for the AES
 * CTR encryption/decryption scheme.
 */
void decrypt_nrf24_payload(
    uint8_t plain_text_payload[],
    uint8_t encrypted_payload[],
    size_t encrypted_payload_size,
    struct AES_ctx *aes_ctx,
    uint8_t aes_iv[]
) {

    // Replace the last four bytes of the AES initialization vector with the last
    // four bytes of the encrypted payload. These unencrypted bytes hold the last
    // four bytes of the initialization vector used with the AES CTR scheme to
    // encrypt the payload.
    memcpy(
        aes_iv + AES_IV_SIZE - AES_IV_COUNTER_SIZE,
        encrypted_payload + encrypted_payload_size - AES_IV_COUNTER_SIZE,
        AES_IV_COUNTER_SIZE
    );

    // Copy the encrypted section of the payload to the plain text buffer.
    memcpy(
        plain_text_payload,
        encrypted_payload,
        encrypted_payload_size - AES_IV_COUNTER_SIZE
    );

    // Update the AES encryption module initialization vector with its new
    // version and decrypt the plain text buffer (which is still encrypted).
    AES_ctx_set_iv(aes_ctx, aes_iv);
    AES_CTR_xcrypt_buffer(
        aes_ctx,
        plain_text_payload,
        encrypted_payload_size - AES_IV_COUNTER_SIZE
    );
}


/**
 * @brief Create an encrypted message from the readings of the sensors.
 * 
 * @param reading pointer to the ambient_info_t structure of sensor readings
 * to include in the radio message.
 * @param aes_ctx pointer to the AES encryption module.
 * @param aes_iv uint8_t array containing the initialization vector for the AES
 * CTR encryption/decryption scheme.
 * @param message uint8_t array where the encrypted message will be stored.
 */
void encrypt_ambient_info_message(
    ambient_info_t *reading,
    struct AES_ctx *aes_ctx,
    uint8_t aes_iv[],
    uint8_t message[]
) {

    float ambient_values_array[sizeof(ambient_info_t) / sizeof(float)] = {
        reading->temperature,
        reading->humidity,
        reading->light_intensity,
        reading->air_pressure,
        reading->air_quality_index
    };

    // Copy each float in the values array byte-to-byte into an auxiliary array.
    uint8_t plain_text_ambient_info_payload[sizeof(ambient_values_array)];
    memcpy(
        plain_text_ambient_info_payload,
        ambient_values_array,
        sizeof(ambient_values_array)
    );

    printf("Original readings: \n");
    printf("Temperature: %f, Humidity: %f\n", reading->temperature, reading->humidity);
    printf("Decrypted plain text bytes: \n");
    for (int i = 0; i < sizeof(ambient_values_array); i++) {
        printf("%x ", plain_text_ambient_info_payload[i]);
    }
    printf("\n\n");
    
    // Encrypt the message using the AES encryption module.
    encrypt_nrf24_payload(
        plain_text_ambient_info_payload,
        sizeof(plain_text_ambient_info_payload),
        message,
        aes_ctx,
        aes_iv,
        &data_retained_in_hibernation.aes_ctr_counter
    );
}
