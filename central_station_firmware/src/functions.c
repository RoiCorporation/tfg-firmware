#include <math.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/rand.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "bme680_port.h"
#include "central_station_firmware.h"
#include "utils.h"
#include "ecdh.h"

#include <stdio.h>


/**
 * @brief Initialize the different station components, such as stdio, GPIO, I2C
 * and the sensors.
 * 
 * @param bme680_sensor pointer to the struct that acts as an instance of the 
 * BME680 sensor.
 * @param bme680_conf pointer to the struct for the BME680 sensor's configuration.
 * @param bme680_heater_conf pointer to the struct for the configuration of the 
 * @param oled_display pointer to the OLED display driver.
 * BME680 sensor's heater.
 * @param nrf24_module pointer to the NRF24L01 module driver.
 * @param ecdh_private_key uint8_t array that will store the station's private key
 * to generate an ECDH public key.
 * @param ecdh_public_key uint8_t array that will store the station's ECDH public
 * key to send in a handshake association attempt with another station.
 * @param associated_wireless_stations_info_map buffer that maps all the necessary
 * information of every wireless station associated with this central one.
 * @param connection_manager pointer to the connection manager.
 * @param copi_pin SPI COPI microcontroller pin.
 * @param cipo_pin CIPO microcontroller pin.
 * @param sck_pin SPI SCK microcontroller pin.
 * @param cs_pin SPI CS microcontroller pin.
 * @param ce_pin SPI CE microcontroller pin.
 * @param spi_baudrate SPI baudrate (in Herz).
 */
void initialize_station(
    struct bme68x_dev *bme680_sensor,
    struct bme68x_conf *bme680_conf,
    struct bme68x_heatr_conf *bme680_heater_conf,
    ssd1306_t *oled_display,
    nrf_client_t *nrf24_module,
    associated_wireless_station_info_t associated_wireless_stations_info_map[],
    uint8_t ecdh_private_key[],
    uint8_t ecdh_public_key[],
    struct mg_mgr *connection_manager,
    uint8_t copi_pin,
    uint8_t cipo_pin,
    uint8_t sck_pin,
    uint8_t cs_pin,
    uint8_t ce_pin,
    uint32_t spi_baudrate
) {
    stdio_init_all();
    initialize_i2c_bus();
    sleep_ms(200);
    initialize_bme680_sensor(bme680_sensor, bme680_conf, bme680_heater_conf);
    oled_display->external_vcc=false;
    ssd1306_init(oled_display, 128, 64, OLED_DISPLAY_I2C_ADDRESS, i2c0);
    sleep_ms(200);
    ssd1306_clear(oled_display);
    initialize_associated_station_info_map(
        associated_wireless_stations_info_map,
        NRF24_ADDRESSES_BUFFER_SIZE,
        NRF24_ADDRESS_SIZE
    );
    initialize_nrf24_module(
        nrf24_module,
        associated_wireless_stations_info_map,
        copi_pin,
        cipo_pin,
        sck_pin,
        cs_pin,
        ce_pin,
        spi_baudrate
    );
    mg_mgr_init(connection_manager);

    
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
 * @param bme680_sensor struct that acts as an instance of the sensor.
 * @param bme680_conf struct for the sensor's configuration.
 * @param bme680_heater_conf struct for the configuration of the sensor's heater.
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
 * @param nrf24_module pointer to the NRF24L01 module driver.
 * @param associated_wireless_stations_info_map buffer that maps all the necessary
 * information of every wireless station associated with this central one.
 * @param copi_pin SPI COPI microcontroller pin.
 * @param cipo_pin SPI CIPO microcontroller pin.
 * @param sck_pin SPI SCK microcontroller pin.
 * @param cs_pin SPI CS microcontroller pin.
 * @param ce_pin SPI CE microcontroller pin.
 * @param spi_baudrate SPI baudrate (in Herz).
 */
void initialize_nrf24_module(
    nrf_client_t *nrf24_module,
    associated_wireless_station_info_t associated_wireless_stations_info_map[],
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
    
    // Declare and initialize the nrf24l01 module.
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

    // Make the first data pipe available to receive incoming handshake requests
    // from wireless stations and initialize the remaining data pipe's addresses.
    nrf24_module->rx_destination(DATA_PIPE_0, (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00});
    for (data_pipe_t pipe = DATA_PIPE_1; pipe < ALL_DATA_PIPES; pipe++) {
        nrf24_module->rx_destination(
            pipe,
            associated_wireless_stations_info_map[pipe].nrf24l01_address
        );
    }

    // Configure the transmit destination for when the station needs to send
    // packets to a wireless station (e.g. during the handshake).
    nrf24_module->tx_destination((uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00});

    // Set to module RX mode and wait for a while to ensure it's entered RX mode.
    nrf24_module->receiver_mode();
    sleep_ms(300);
}


/**
 * @brief Implements the handshake protocol necessary to associate a wireless
 * station with this one.
 * 
 * @param nrf24_module pointer to the NRF24L01 module driver.
 * @param ecdh_private_key uint8_t array that stores the station's ECDH private
 * key.
 * @param ecdh_public_key uint8_t array that stores the station's ECDH public
 * key that gets sent to the other station at the beginning of the handshake.
 * @param kdf_salt uint8_t array containing the salt used in the KDF.
 * @param aes_iv uint8_t array containing the initialization vector with which
 * to set up the AES encryption module.
 * @param associated_wireless_stations_info_map buffer that maps all the necessary
 * information of every wireless station associated with this central one.
 * @param buffer_size size of the buffer that maps each station ID with their
 * corresponding NRF24 module address. 
 * @return int8_t 0 if the handshake was completed successfully, -1 otherwise.
 */
int8_t handshake(
    nrf_client_t *nrf24_module,
    uint8_t ecdh_private_key[],
    uint8_t ecdh_public_key[],
    uint8_t kdf_salt[],
    uint8_t aes_iv[],
    associated_wireless_station_info_t associated_wireless_stations_info_map[],
    size_t buffer_size
) {

    printf("ENTERED HANDSHAKE\n");

    // Variables used in the method.
    int read_loop_iterations = 0;
    int8_t mappings_buffer_modified = -1;
    uint8_t data_pipe_read = 0;
    data_pipe_t first_available_data_pipe = ALL_DATA_PIPES;
    uint8_t other_station_ecdh_public_key[ECC_PUB_KEY_SIZE] = {0x00};
    uint8_t aux_pub_key_first_half_buffer[ECC_PUB_KEY_SIZE / 2] = {0x00};
    uint8_t aux_pub_key_second_half_buffer[ECC_PUB_KEY_SIZE / 2] = {0x00};
    uint8_t shared_secret[ECC_PUB_KEY_SIZE] = {0x00};
    uint8_t aes_key[AES_KEY_SIZE] = {0x00};
    uint8_t wireless_station_id_bytes[STATION_ID_BYTES_LENGTH] = {0x00};
    uint8_t nrf24_address_available[NRF24_ADDRESS_SIZE] = {0x00};
    uint8_t nrf24_address_packet[NRF24_ADDRESS_SIZE + AES_IV_COUNTER_SIZE] = {0x00};
    uint8_t check_nrf24_address[NRF24_ADDRESS_SIZE] = {0x00};
    uint8_t aux_flush_rx_fifo_buffer[NRF24_MAX_PACKET_SIZE] = {0x00};
    uint8_t handshake_result_packet[NRF24_MAX_PACKET_SIZE] = {0x00};
    uint8_t handshake_result[NRF24_MAX_PACKET_SIZE - AES_IV_COUNTER_SIZE] = {0x00};
    uint8_t handshake_fail[NRF24_MAX_PACKET_SIZE - AES_IV_COUNTER_SIZE] = {0xff};
    
    // Search the first available data pipe.
    for (int i = 0; i < buffer_size; i++) {
        if (associated_wireless_stations_info_map[i].station_id == NULL) {
            first_available_data_pipe = i;
            break;
        }
    }
    // The handshake will fail if there are no remaining data pipes available.
    if (first_available_data_pipe == ALL_DATA_PIPES)
        return (int8_t)-1;

    /* --------------------------------------------------------------------- */
    // Receive the first half of the wireless station's ECDH public key.
    
    while (1) {

        // Check if any packets have been received.
        if (nrf24_module->is_packet(&data_pipe_read)) {

            // If the address of the received packet corresponds to another
            // data pipe, read it to force a flush of the RX FIFO and allow
            // actual handshake-related packets to enter such FIFO.
            if (data_pipe_read != first_available_data_pipe) {
                nrf24_module->read_packet(
                    aux_flush_rx_fifo_buffer,
                    sizeof(aux_flush_rx_fifo_buffer)
                );
            }

            // If a packet is successfully read on the current available data
            // pipe, save that packet as the first half of the wireless station's
            // ECDH public key.
            else {
                if (nrf24_module->read_packet(
                    aux_pub_key_first_half_buffer,
                    sizeof(aux_pub_key_first_half_buffer)
                ) != ERROR)
                    break;
            }
        }
        read_loop_iterations++;
        if (read_loop_iterations >= HANDSHAKE_MAX_READ_LOOP_ITERATIONS) {
            exit_handshake(
                nrf24_module,
                associated_wireless_stations_info_map,
                buffer_size,
                data_pipe_read,
                mappings_buffer_modified
            );
            return (int8_t)-1;
        }

        sleep_ms(10);
    }
    read_loop_iterations = 0;

    /* --------------------------------------------------------------------- */
    // Send the first half of this station's ECDH public key.

    // Set the nrf24 module to standby-I mode to prepare it to enter TX mode and
    // wait for a while to ensure it's entered TX mode.
    nrf24_module->standby_mode();
    sleep_ms(50);

    // Send the packet with the first half of the station's ECDH public key.
    for (int i = 0; i < HANDSHAKE_RETRANSMISSIONS; i++) {
        nrf24_module->send_packet(ecdh_public_key, ECC_PUB_KEY_SIZE / 2);
        sleep_ms(5);
    }

    /* --------------------------------------------------------------------- */
    // Receive the second half of the wireless station's ECDH public key.

    // Set to module RX mode and wait for a while to ensure it's entered RX mode.
    nrf24_module->receiver_mode();
    sleep_ms(20);

    while (1) {

        // Check if any packets have been received.
        if (nrf24_module->is_packet(&data_pipe_read)) {

            // If the address of the received packet corresponds to another
            // data pipe, read it to force a flush of the RX FIFO and allow
            // actual handshake-related packets to enter such FIFO.
            if (data_pipe_read != first_available_data_pipe) {
                nrf24_module->read_packet(aux_flush_rx_fifo_buffer, sizeof(aux_flush_rx_fifo_buffer));
            }

            // If a packet is successfully read on the current available data
            // pipe, save that packet as the second half of the wireless station's
            // ECDH public key.
            else {
                if (nrf24_module->read_packet(
                    aux_pub_key_second_half_buffer,
                    sizeof(aux_pub_key_second_half_buffer)
                ) != ERROR)
                    break;
            }
        }
        read_loop_iterations++;
        if (read_loop_iterations >= HANDSHAKE_MAX_READ_LOOP_ITERATIONS) {
            exit_handshake(
                nrf24_module,
                associated_wireless_stations_info_map,
                buffer_size,
                data_pipe_read,
                mappings_buffer_modified
            );
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
    sleep_ms(50);

    // Send the packet with the second half of the station's ECDH public key.
    for (int i = 0; i < HANDSHAKE_RETRANSMISSIONS; i++) {
        nrf24_module->send_packet(ecdh_public_key + ECC_PUB_KEY_SIZE / 2, ECC_PUB_KEY_SIZE / 2);
        sleep_ms(5);
    }
    
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
        shared_secret) == 0)
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

    // Initialize the AES encryption module of the wireless station info
    // map entry that corresponds to the data pipe were messages for this
    // handshake are being received.
    AES_init_ctx_iv(
        &associated_wireless_stations_info_map[data_pipe_read].aes_ctx,
        aes_key,
        aes_iv
    );

    /* --------------------------------------------------------------------- */
    // Receive and decrypt the wireless station's ID.

    // Set to module RX mode and wait for a while to ensure it's entered RX mode.
    nrf24_module->receiver_mode();
    sleep_ms(20);

    // Flush the NRF24L01 RX FIFO to prevent the NRF24 module from rejecting
    // inbound messages.
    nrf24_module->read_packet(
        aux_flush_rx_fifo_buffer,
        sizeof(aux_flush_rx_fifo_buffer)
    );

    while (1) {
        // Check if any packets have been received.
        if (nrf24_module->is_packet(&data_pipe_read)) {

            // If the address of the received packet corresponds to another
            // data pipe, read it to force a flush of the RX FIFO and allow
            // actual handshake-related packets to enter such FIFO.
            if (data_pipe_read != first_available_data_pipe) {
                nrf24_module->read_packet(
                    aux_flush_rx_fifo_buffer,
                    sizeof(aux_flush_rx_fifo_buffer)
                );
            }

            // If a packet is successfully read on the current available data
            // pipe, store that packet as the wireless station id (still in
            // bytes, not yet in string format). Then, configure that data pipe
            // so that it only receives packets in that specific address.
            else {
                if (nrf24_module->read_packet(
                    wireless_station_id_bytes, sizeof(wireless_station_id_bytes)) != ERROR) 
                {
            
                    AES_CTR_xcrypt_buffer(
                        &associated_wireless_stations_info_map[data_pipe_read].aes_ctx,
                        wireless_station_id_bytes,
                        sizeof(wireless_station_id_bytes)
                    );
                    mappings_buffer_modified = 0;
                    if (first_available_data_pipe < DATA_PIPE_2) {
                        nrf24_module->rx_destination(
                            first_available_data_pipe,
                            associated_wireless_stations_info_map[first_available_data_pipe].nrf24l01_address
                        );
                    }
                    else {
                        nrf24_module->rx_destination(
                            first_available_data_pipe,
                            &associated_wireless_stations_info_map[first_available_data_pipe].nrf24l01_address[NRF24_ADDRESS_SIZE - 1]
                        );
                    }
                    data_pipe_t next_available_data_pipe = first_available_data_pipe + 1;
                    if (next_available_data_pipe < ALL_DATA_PIPES) {
                        if (next_available_data_pipe < DATA_PIPE_2) {
                            nrf24_module->rx_destination(
                                next_available_data_pipe,
                                (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00}
                            );
                        }
                        else {
                            nrf24_module->rx_destination(
                                next_available_data_pipe,
                                (uint8_t[]){0x00}
                            );
                        }
                    }
                    break;
                }
            }
        }
        read_loop_iterations++;
        if (read_loop_iterations >= HANDSHAKE_MAX_READ_LOOP_ITERATIONS) {
            exit_handshake(
                nrf24_module,
                associated_wireless_stations_info_map,
                buffer_size,
                data_pipe_read,
                mappings_buffer_modified
            );
            return (int8_t)-1;
        }

        sleep_ms(20);
    }
    read_loop_iterations = 0;

    // Convert the received station ID from a byte array to a proper,
    // UUID-formatted string ID and store it into the id_to_address
    // buffer entry mapped to the aforementioned data pipe.
    int8_t result_uuid_to_string_conversion = bytes_to_string_uuid(
        wireless_station_id_bytes,
        &associated_wireless_stations_info_map[first_available_data_pipe].station_id,
        STATION_ID_BYTES_LENGTH,
        STATION_ID_CHAR_LENGTH
    );

    if (result_uuid_to_string_conversion != 0)
        return result_uuid_to_string_conversion;

    /* --------------------------------------------------------------------- */
    // Send to the wireless station the new NRF24L01 address to which it
    // should send its readings going forward.

    // Set the nrf24 module to standby-I mode to prepare it to enter TX mode and
    // wait for a while to ensure it's entered TX mode.
    nrf24_module->standby_mode();
    sleep_ms(300);

    // Copy the address of the corresponding data pipe to the packet array. 
    // This is the payload of the packet that the nrf24 module will send.
    memcpy(
        nrf24_address_available,
        associated_wireless_stations_info_map[first_available_data_pipe].nrf24l01_address,
        NRF24_ADDRESS_SIZE
    );

    // Encrypt the message using the AES encryption module set up above.
    encrypt_nrf24_payload(
        nrf24_address_available,
        sizeof(nrf24_address_available),
        nrf24_address_packet,
        &associated_wireless_stations_info_map[data_pipe_read].aes_ctx,
        aes_iv,
        &associated_wireless_stations_info_map[first_available_data_pipe].aes_ctr_counter
    );

    printf("\n\n");
    printf("Original message: \n");
    for (int i = 0; i < sizeof(nrf24_address_available); i++) {
        printf("%x, ", nrf24_address_available[i]);
    }
    printf("\n");
    printf("Encrypted (right before sending): \n");
    for (int i = 0; i < sizeof(nrf24_address_packet); i++) {
        printf("%x, ", nrf24_address_packet[i]);
    }

    // Send the packet.
    for (int i = 0; i < HANDSHAKE_RETRANSMISSIONS; i++) {
        nrf24_module->send_packet(
            nrf24_address_packet,
            sizeof(nrf24_address_packet)
        );
        sleep_ms(15);
    }

    /* --------------------------------------------------------------------- */
    // Receive the NRF24L01 address that the wireless station got to ensure
    // it matches the one sent by this station.

    // Set to module RX mode and wait for a while to ensure it's entered RX mode.
    nrf24_module->receiver_mode();
    sleep_ms(30);

    // Flush the NRF24L01 RX FIFO to prevent the NRF24 module from rejecting
    // inbound messages.
    nrf24_module->read_packet(
        aux_flush_rx_fifo_buffer,
        sizeof(aux_flush_rx_fifo_buffer)
    );

    while (1) {

        // Check if any packets have been received.
        if (nrf24_module->is_packet(&data_pipe_read)) {

            // If the address of the received packet corresponds to another
            // data pipe, read it to force a flush of the RX FIFO and allow
            // actual handshake-related packets to enter such FIFO.
            if (data_pipe_read != first_available_data_pipe) {
                nrf24_module->read_packet(
                    aux_flush_rx_fifo_buffer,
                    sizeof(aux_flush_rx_fifo_buffer)
                );
            }

            // If a packet is successfully read on the current available data
            // pipe, store that packet as the wireless station's destination
            // NRF24L01 address.
            else {
                if (nrf24_module->read_packet(
                    nrf24_address_packet, 
                    sizeof(nrf24_address_packet)
                ) != ERROR) 
                    break;
            }
        }
        read_loop_iterations++;
        if (read_loop_iterations >= HANDSHAKE_MAX_READ_LOOP_ITERATIONS) {
            exit_handshake(
                nrf24_module,
                associated_wireless_stations_info_map,
                buffer_size,
                data_pipe_read,
                mappings_buffer_modified
            );
            return (int8_t)-1;
        }

        sleep_ms(20);
    }
    read_loop_iterations = 0;

    // Decrypt the message containing the NRF24 destination address that
    // the wireless station received using the AES encryption module set
    // up above.
    decrypt_nrf24_payload(
        nrf24_address_available,
        nrf24_address_packet,
        sizeof(nrf24_address_packet),
        &associated_wireless_stations_info_map[data_pipe_read].aes_ctx,
        aes_iv
    );

    printf("Decrypted address received: \n");
    for (int i = 0; i < sizeof(nrf24_address_available); i++) {
        printf("%x, ", nrf24_address_available[i]);
    }
    printf("\n");

    /* --------------------------------------------------------------------- */
    // Compare the NRF24 address received by the wireless station and the one
    // sent by this station and send the result to the wireless station.

    // Set the nrf24 module to standby-I mode to prepare it to enter TX mode and
    // wait for a while to ensure it's entered TX mode.
    nrf24_module->standby_mode();
    sleep_ms(300);

    // Compare the two addresses.
    int comparison_result = memcmp(
        nrf24_address_available,
        associated_wireless_stations_info_map[first_available_data_pipe].nrf24l01_address,
        NRF24_ADDRESS_SIZE
    );

    // If the addresses don't match, write a failure message in the handshake
    // result buffer.
    if (comparison_result != 0) {
        memcpy(
            handshake_result,
            handshake_fail,
            NRF24_MAX_PACKET_SIZE - AES_IV_COUNTER_SIZE
        );
    }

    // Encrypt the result using the AES encryption module set up above.
    encrypt_nrf24_payload(
        handshake_result,
        sizeof(handshake_result),
        handshake_result_packet,
        &associated_wireless_stations_info_map[data_pipe_read].aes_ctx,
        aes_iv,
        &associated_wireless_stations_info_map[first_available_data_pipe].aes_ctr_counter
    );

    printf("\n\n");
    printf("Original message: \n");
    for (int i = 0; i < sizeof(handshake_result); i++) {
        printf("%x, ", handshake_result[i]);
    }
    printf("\n");
    printf("Encrypted (right before sending): \n");
    for (int i = 0; i < sizeof(handshake_result_packet); i++) {
        printf("%x, ", handshake_result_packet[i]);
    }
    
    // Send the packet.
    for (int i = 0; i < HANDSHAKE_RETRANSMISSIONS; i++) {
        nrf24_module->send_packet(
            handshake_result_packet,
            sizeof(handshake_result_packet)
        );
        sleep_ms(15);
    }

    // Set to module RX mode and wait for a while to ensure it's entered RX mode.
    nrf24_module->receiver_mode();
    sleep_ms(30);
    
    // If the address comparison wasn't successful, exit the handshake returning
    // an error. Otherwise, the handshake was completed successfully.
    if (comparison_result != 0)
        return (int8_t)-1;
    return (int8_t)0;
}


/**
 * @brief Handle the exit of the handshake protocol on failure, resetting the
 * configuration of the NRF24L01 module to its original setup and clearing any
 * entry on the associated wireless info buffer that might have been updated
 * during the handshake.
 * 
 * @param nrf24_module pointer to the NRF24L01 module driver.
 * @param associated_wireless_stations_info_map buffer that maps all the necessary
 * information of every wireless station associated with this central one.
 * @param buffer_size size of the buffer that maps each station ID with their
 * corresponding NRF24 module address.
 * @param data_pipe_read the index of the mappings buffer entry that was modified.
 * @param mappings_buffer_modified 0 if the mappings buffer was actually modified,
 * -1 otherwise.
 */
void exit_handshake(
    nrf_client_t *nrf24_module,
    associated_wireless_station_info_t associated_wireless_stations_info_map[],
    size_t buffer_size,
    uint8_t data_pipe_read,
    int8_t mappings_buffer_modified
) {

    if (mappings_buffer_modified == 0) {

        // Reset the AES CTR counter and station ID of the mappings buffer
        // entry that was modified during the handshake.
        associated_wireless_stations_info_map[data_pipe_read].station_id = NULL;
        associated_wireless_stations_info_map[data_pipe_read].aes_ctr_counter = 0;

        // Reset the destination addresses of the NRF24L01 that were modified
        // during the handshake.
        if (data_pipe_read < DATA_PIPE_2) {
            nrf24_module->rx_destination(
                data_pipe_read,
                (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00}
            );
        }
        else
            nrf24_module->rx_destination(data_pipe_read, (uint8_t[]){0x00});

        data_pipe_t next_available_data_pipe = data_pipe_read + 1;
        if (next_available_data_pipe < ALL_DATA_PIPES) {
            if (next_available_data_pipe < DATA_PIPE_2) {
                nrf24_module->rx_destination(
                    next_available_data_pipe,
                    associated_wireless_stations_info_map[next_available_data_pipe].nrf24l01_address
                );
            }
            else {
                nrf24_module->rx_destination(
                    next_available_data_pipe,
                    &associated_wireless_stations_info_map[next_available_data_pipe].nrf24l01_address[NRF24_ADDRESS_SIZE - 1]
                );
            }
        }
    }
    
    // Set the TX address to be the default handshake address that all
    // central stations listen to.
    nrf24_module->tx_destination((uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00});

    // Set to module RX mode and wait for a while to ensure it's entered RX mode.
    nrf24_module->receiver_mode();
    sleep_ms(300);
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

    // If successful, calculate the actual light intensity in lux.
    // Otherwise, return -1.
    if (bytes_read == 2) {
        uint16_t raw = (buf[0] << 8) | buf[1];
        float lux = raw / 1.2f;
        reading->light_intensity = lux;
        return (int8_t)0;
    } 
    
    return (int8_t)-1;
}


/**
 * @brief Receive and store an encrypted radio message that contains the
 * sensor readings of an associated wireless station. Also saves the data
 * pipe of the NRF24L01 module where the packet was received, to later identify
 * which wireless station sent the readings.
 * 
 * @param nrf24_module pointer to the NRF24L01 module driver.
 * @param message uint8_t array where the received encrypted message is stored.
 * @param incoming_packet_data_pipe pointer to an uint8_t that will be assigned
 * with the data pipe number of the incoming packet.
 * @return int8_t 0 if the radio message was received successfully, -1 otherwise.
 */
int8_t receive_station_readings(
    nrf_client_t *nrf24_module,
    uint8_t message[],
    size_t message_size,
    uint8_t *incoming_packet_data_pipe
) {
    while (1) {
        if (nrf24_module->is_packet(incoming_packet_data_pipe)) {            
            if (nrf24_module->read_packet(message, message_size) != ERROR) {
                return (int8_t)0;
            }
            return (int8_t)-1;
        }
        return (int8_t)-1;
    }
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

    printf("DECRYPTED payload: ");
    for (int i = 0; i < encrypted_payload_size - AES_IV_COUNTER_SIZE; i++) {
        printf("%x ", plain_text_payload[i]);
    }
    printf("\n");
}


/**
 * @brief Decrypt the received radio message and store the obtained sensor 
 * readings in the appropriate structure.
 * 
 * @param received_readings pointer to the sensor readings structure where
 * the decrypted readings will be stored.
 * @param aes_ctx pointer to the AES encryption module.
 * @param aes_iv uint8_t array containing the initialization vector for the AES
 * CTR encryption/decryption scheme.
 * @param message uint8_t array containing the encrypted message as received by
 * the radio module.
 */
void decrypt_ambient_info_message(
    ambient_info_t *received_readings,
    struct AES_ctx *aes_ctx,
    uint8_t aes_iv[],
    uint8_t message[]
) {
    float values_array[WIRELESS_STATION_DATA_FIELD_COUNT];
    uint8_t plain_text_ambient_info_payload[sizeof(float) * WIRELESS_STATION_DATA_FIELD_COUNT];

    // Decrypt the message containing the readings of a wireless station using
    // the AES encryption module.
    decrypt_nrf24_payload(
        plain_text_ambient_info_payload,
        message,
        sizeof(float) * WIRELESS_STATION_DATA_FIELD_COUNT + AES_IV_COUNTER_SIZE,
        aes_ctx,
        aes_iv
    );

    printf("Decrypted plain text bytes: \n");
    for (int i = 0; i < sizeof(plain_text_ambient_info_payload); i++) {
        printf("%x ", plain_text_ambient_info_payload[i]);
    }
    printf("\n\n");

    // Copy the decrypted payload containing the representation in bytes of the
    // station readings in an array of floats.
    memcpy(
        values_array,
        plain_text_ambient_info_payload,
        sizeof(float) * WIRELESS_STATION_DATA_FIELD_COUNT
    );

    // Assign the floats in array to their corresponding fields in the readings
    // structure.
    received_readings->temperature = values_array[0];
    received_readings->humidity = values_array[1];
    received_readings->light_intensity = values_array[2];
    received_readings->air_pressure = values_array[3];
    received_readings->air_quality_index = values_array[4];
}
