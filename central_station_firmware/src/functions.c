#include <math.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "nrf24_driver.h"
#include "aes.h"
#include "central_station_firmware.h"


/**
 * @brief Initialize the different board components, such as stdio, I2C and GPIO.
 */
void initialize_board(
    nrf_client_t* nrf24_module,
    uint8_t copi_pin,
    uint8_t cipo_pin,
    uint8_t sck_pin,
    uint8_t cs_pin,
    uint8_t ce_pin,
    uint32_t spi_baudrate
) {
    stdio_init_all();
    initialize_i2c_bus();
    initialize_nrf24_module(
        nrf24_module, copi_pin, cipo_pin, sck_pin, cs_pin, ce_pin, spi_baudrate
    );
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
 * @brief Configure the nrf24l01 module.
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
){
    
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
    nrf24_module->configure(&nrf24_pins, 7000000);
    
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

    // Address to which the wireless stations will send their packets to.
    nrf24_module->rx_destination(DATA_PIPE_1, (uint8_t[]){0xC7, 0xC7, 0xC7, 0xC7, 0xC7});

    // Set to module RX Mode.
    nrf24_module->receiver_mode();
}


/**
 * @brief Read temperature and humidity from the DHT22 sensor.
 * 
 * @param reading pointer to an ambient_info_t struct where the read values 
 * will be stored.
 * @return int8_t 0 if the reading was successful, else -1.
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
 * @brief Read light intensity from the light sensor.
 * 
 * @param reading pointer to an ambient_info_t struct where the read values 
 * will be stored.
 * @return int8_t 0 if the reading was successful, else -1.
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
  * sensor readings of a linked wireless station.
  * 
  * @param nrf24_module instance of the nrf24l01 driver with which the packet 
  * will be received.
  * @param message array where the received message is stored.
  * @return int8_t 0 if the radio message was received successfully, else -1.
  */
int8_t receive_radio_message(nrf_client_t nrf24_module, uint8_t message[]) {
    
    while (1) {
        if (nrf24_module.is_packet(NULL)) {            
            if (nrf24_module.read_packet(message, sizeof(ambient_info_t)) != 0) {
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
