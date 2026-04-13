#include <math.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "bme680_port.h"
#include "central_station_firmware.h"
#include "utils.h"


#include <stdio.h>


/**
 * @brief Initialize the different station components, such as stdio, GPIO, I2C
 * and the sensors.
 * 
 * @param bme680_sensor pointer to the struct that acts as an instance of the 
 * BME680 sensor.
 * @param bme680_conf pointer to the struct for the BME680 sensor's configuration.
 * @param bme680_heater_conf pointer to the struct for the configuration of the 
 * BME680 sensor's heater.
 * @param nrf24_module pointer to the NRF24L01 module driver.
 * @param station_id_to_nrf24_address_buffer buffer that maps the associated
 * wireless stations ID's with the NRF24L01 module data pipe addresses.
 * @param oled_display pointer to the OLED display driver.
 * @param connection_manager pointer to the connection manager.
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
    nrf_client_t* nrf24_module,
    station_id_address_map_t station_id_to_nrf24_address_buffer[],
    ssd1306_t* oled_display,
    struct mg_mgr* connection_manager,
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
    sleep_ms(100);
    initialize_bme680_sensor(bme680_sensor, bme680_conf, bme680_heater_conf);
    initialize_station_id_to_address_buffer(
        station_id_to_nrf24_address_buffer,
        NRF24_ADDRESSES_BUFFER_SIZE,
        NRF24_ADDRESS_SIZE
    );
    initialize_nrf24_module(
        nrf24_module,
        station_id_to_nrf24_address_buffer,
        copi_pin,
        cipo_pin,
        sck_pin,
        cs_pin,
        ce_pin,
        spi_baudrate
    );
    ssd1306_init(oled_display, 128, 64, 0x3c, i2c0);
    ssd1306_clear(oled_display);
    oled_display->external_vcc=false;
    mg_mgr_init(connection_manager);
    gpio_init(DHT22_PIN);
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
 * @param nrf24_module pointer to the NRF24L01 module driver.
 * @param copi_pin SPI COPI microcontroller pin.
 * @param cipo_pin SPI CIPO microcontroller pin.
 * @param sck_pin SPI SCK microcontroller pin.
 * @param cs_pin SPI CS microcontroller pin.
 * @param ce_pin SPI CE microcontroller pin.
 * @param spi_baudrate SPI baudrate (in Herz).
 */
void initialize_nrf24_module(
    nrf_client_t* nrf24_module,
    station_id_address_map_t station_id_to_nrf24_address_buffer[],
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
        .retr_count = ARC_2RT,          // 2 packet retransmissions.
        .retr_delay = ARD_500US         // 500μS retransmission delay.
    };
    
    nrf24_module->initialise(&nrf24_config);
    // nrf24_module->initialise(NULL);
    // nrf24_module->dyn_payloads_enable();

    // Make the first data pipe available to receive incoming handshake requests
    // from wireless stations and initialize the remaining data pipe's addresses.
    nrf24_module->rx_destination(DATA_PIPE_0, (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00});
    for (data_pipe_t pipe = DATA_PIPE_1; pipe < ALL_DATA_PIPES; pipe++) {
        nrf24_module->rx_destination(pipe, station_id_to_nrf24_address_buffer[pipe].nrf24l01_address);
    }

    // Configure the transmit destination for when the station needs to send
    // packets to a wireless station (e.g. during the handshake).
    nrf24_module->tx_destination((uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00});

    // Set to module RX Mode.
    nrf24_module->receiver_mode();
}


/**
 * @brief Handles the handshake protocol necessary to establish a connection
 * between this station and a new wireless station.
 * 
 * @param nrf24_module pointer to the NRF24L01 module driver.
 * @param station_id_to_nrf24_address_buffer character array where the ID of the
 * wireless station will be stored.
 * @param buffer_size size of the buffer that maps each station ID with their
 * corresponding nrf24 module address. 
 * @return int8_t 0 if the handshake was completed successfully, -1 otherwise.
 */
int8_t handshake(
    nrf_client_t *nrf24_module,
    station_id_address_map_t station_id_to_nrf24_address_buffer[],
    size_t buffer_size
) {

    // Variables used in the method.
    uint8_t data_pipe_read = 0;
    data_pipe_t first_available_data_pipe = ALL_DATA_PIPES;
    uint8_t wireless_station_id_bytes[STATION_ID_BYTES_LENGTH];
    uint8_t packet_to_send[NRF24_ADDRESS_SIZE];
    
    // Search the first available data pipe.
    for (int i = 0; i < buffer_size; i++) {
        if (station_id_to_nrf24_address_buffer[i].associated_station_id == NULL) {
            first_available_data_pipe = i;
            break;
        }
    }
    // The handshake will fail if there are no remaining data pipes available.
    if (first_available_data_pipe == ALL_DATA_PIPES)
        return (int8_t)-1;

    while (1) {
        // If a packet is successfully read on the current available data pipe,
        // store that packet as the wireless station id (still in bytes, not yet
        // in string format). Then, configure that data pipe so that it only
        // receives packets in that specific address.
        if (nrf24_module->is_packet(&data_pipe_read)) {
            if (data_pipe_read == first_available_data_pipe) {
                if (nrf24_module->read_packet(
                    wireless_station_id_bytes, sizeof(wireless_station_id_bytes)) != ERROR) 
                {
                    if (first_available_data_pipe < DATA_PIPE_2)
                        nrf24_module->rx_destination(first_available_data_pipe, station_id_to_nrf24_address_buffer[first_available_data_pipe].nrf24l01_address);
                    else
                        nrf24_module->rx_destination(first_available_data_pipe, &station_id_to_nrf24_address_buffer[first_available_data_pipe].nrf24l01_address[NRF24_ADDRESS_SIZE - 1]);
                    data_pipe_t next_available_data_pipe = first_available_data_pipe + 1;
                    if (next_available_data_pipe < ALL_DATA_PIPES) {
                        if (next_available_data_pipe < 2)
                            nrf24_module->rx_destination(next_available_data_pipe, (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00});
                        else
                            nrf24_module->rx_destination(next_available_data_pipe, (uint8_t[]){0x00});
                    }
                    break;
                }
            }
        }
        sleep_ms(200);
    }

    // Convert the received station ID from a byte array to a proper,
    // UUID-formatted string ID and store it into the id_to_address
    // buffer entry mapped to the aforementioned data pipe.
    int8_t result_uuid_to_string_conversion = bytes_to_string_uuid(
        wireless_station_id_bytes,
        &station_id_to_nrf24_address_buffer[first_available_data_pipe].associated_station_id,
        STATION_ID_BYTES_LENGTH,
        STATION_ID_CHAR_LENGTH
    );
    
    if (result_uuid_to_string_conversion != 0)
        return result_uuid_to_string_conversion;

    /* --------------------------------------------------------------------- */
    // Now the central station has to transmit to the wireless station the new
    // address to which it should send its readings going forward.

    // Set the nrf24 module to standby-I mode to prepare it to enter TX mode and
    // wait for a while to ensure it's entered TX mode.
    nrf24_module->standby_mode();
    sleep_ms(30);

    // Copy the address of the corresponding data pipe to the packet array. 
    // This is the payload of the packet that the nrf24 module will send.
    memcpy(
        packet_to_send,
        station_id_to_nrf24_address_buffer[first_available_data_pipe].nrf24l01_address,
        NRF24_ADDRESS_SIZE
    );

    // Send the packet.
    nrf24_module->send_packet(packet_to_send, sizeof(packet_to_send));

    // Set to module RX Mode and wait for a while to ensure it's entered RX mode.
    nrf24_module->receiver_mode();
    sleep_ms(30);
    
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
 * @brief Read temperature and humidity from the DHT22 sensor.
 * 
 * @param reading pointer to an ambient_info_t struct where the read values 
 * will be stored.
 * @return int8_t 0 if the reading was successful, -1 otherwise.
 */
int8_t read_temperature_and_humidity(ambient_info_t *reading) {
    int data[5] = {0, 0, 0, 0, 0};
    uint j = 0;
    uint last = 1;

    // Send start signal.
    gpio_set_dir(DHT22_PIN, GPIO_OUT);
    gpio_put(DHT22_PIN, 0);
    sleep_ms(20);
    gpio_set_dir(DHT22_PIN, GPIO_IN);

    for (uint i = 0; i < MAX_TIMINGS; i++) {
        // Measure pulse length in µs,
        uint32_t start_time = time_us_32();
        while (gpio_get(DHT22_PIN) == last) {
            if ((time_us_32() - start_time) > 1000) break; // 1 ms timeout,
        }
        uint32_t pulse_length = time_us_32() - start_time;
        last = gpio_get(DHT22_PIN);

        if (pulse_length > 1000) break; // Sensor stopped responding.

        // Starting from bit 0 after ~4 transitions.
        if ((i >= 4) && (i % 2 == 0)) {
            data[j / 8] <<= 1;
            if (pulse_length > 35) data[j / 8] |= 1;  // >35 µs = 1, else 0.
            j++;
        }
    }

    // Verify checksum. If successful, return 0, else return -1.
    if ((j >= 40) &&
        (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF))) {

        reading->humidity = (float) ((data[0] << 8) + data[1]) / 10;
        if (reading->humidity > 100) reading->humidity = data[0]; // DHT11 fallback.

        reading->temperature = (float) (((data[2] & 0x7F) << 8) + data[3]) / 10;
        if (reading->temperature > 125) reading->temperature = data[2];

        if (data[2] & 0x80) reading->temperature = -reading->temperature; // Negative temp.

        return (int8_t)0;
    }

    reading->temperature = NAN;
    reading->humidity = NAN;
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
 * @brief Receive and store an encrypted radio message that contains the
 * sensor readings of an associated wireless station. Also saves the data
 * pipe of the nrf24 module where the packet was received, to later identify
 * which wireless station sent the readings.
 * 
 * @param nrf24_module instance of the nrf24l01 driver with which the packet
 * will be received.
 * @param message array where the received message is stored.
 * @param incoming_packet_data_pipe pointer to an uint8_t that will be assigned
 * with the data pipe number of the incoming packet.
 * @return int8_t 0 if the radio message was received successfully, -1 otherwise.
 */
int8_t receive_radio_message(
    nrf_client_t nrf24_module,
    uint8_t message[],
    uint8_t *incoming_packet_data_pipe
) {
    while (1) {
        if (nrf24_module.is_packet(incoming_packet_data_pipe)) {            
            if (nrf24_module.read_packet(message, sizeof(ambient_info_t)) != ERROR) {
                return (int8_t)0;
            }
            return (int8_t)-1;
        }
        return (int8_t)-1;
    }
}


/**
 * @brief Decrypt the received radio message and store the obtained sensor 
 * readings in the appropriate structure.
 * 
 * @param received_readings pointer to the sensor readings structure where
 * the decrypted readings will be stored.
 * @param aes_ctx AES decryption engine context.
 * @param aes_key decryption key needed to decrypt the message.
 * @param aes_iv initialization vector with which the original message was
 * encrypted.
 * @param message encrypted message as received by the radio module.
 */
void decrypt_ambient_info_message(
    ambient_info_t *received_readings,
    struct AES_ctx aes_ctx,
    const uint8_t aes_key[],
    const uint8_t aes_iv[],
    uint8_t message[]
) {
    float values_array[sizeof(ambient_info_t) / sizeof(float)];

    AES_init_ctx_iv(&aes_ctx, aes_key, aes_iv);
    AES_CTR_xcrypt_buffer(&aes_ctx, message, sizeof(ambient_info_t));

    memcpy(values_array, message, sizeof(ambient_info_t));

    received_readings->temperature = values_array[0];
    received_readings->humidity = values_array[1];
    received_readings->light_intensity = values_array[2];
    received_readings->air_pressure = values_array[3];
    received_readings->air_quality_index = values_array[4];
}
