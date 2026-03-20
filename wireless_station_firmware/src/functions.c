#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "nrf24_driver.h"
#include "wireless_station_firmware.h"


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
        // RF Channel 
        .channel = 120,

        // AW_3_BYTES, AW_4_BYTES, AW_5_BYTES
        .address_width = AW_5_BYTES,

        // dynamic payloads: DYNPD_ENABLE, DYNPD_DISABLE
        .dyn_payloads = DYNPD_ENABLE,

        // data rate: RF_DR_250KBPS, RF_DR_1MBPS, RF_DR_2MBPS
        .data_rate = RF_DR_250KBPS,

        // RF_PWR_NEG_18DBM, RF_PWR_NEG_12DBM, RF_PWR_NEG_6DBM, RF_PWR_0DBM
        .power = RF_PWR_NEG_12DBM,

        // retransmission count: ARC_NONE...ARC_15RT
        .retr_count = ARC_2RT,

        // retransmission delay: ARD_250US, ARD_500US, ARD_750US, ARD_1000US
        .retr_delay = ARD_500US 
    };
    
    nrf24_module->initialise(&nrf24_config);
    // nrf24_module->initialise(NULL);
    // nrf24_module->dyn_payloads_enable();

    // Set to Standby-I Mode.
    nrf24_module->standby_mode();
}


/**
 * @brief Read temperature and humidity from the DHT22 sensor.
 * 
 * @param reading pointer to an ambient_info_t struct where the read values will be stored.
 * @return int 0 if the reading was successful, else -1.
 */
int read_temperature_and_humidity(ambient_info_t *reading) {
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

        return 0;
    }

    reading->temperature = NAN;
    reading->humidity = NAN;
    return -1;
}


/**
 * @brief Read light intensity from the light sensor.
 * 
 * @param reading pointer to an ambient_info_t struct where the read values will be stored.
 * @return int 0 if the reading was successful, else -1.
 */
int read_light_intensity(ambient_info_t *reading) {
    // Tries to read 2 bytes.
    uint8_t buf[2];
    int bytes_read = i2c_read_blocking(i2c0, LIGHT_SENSOR_I2C_ADDRESS, buf, 2, false);

    // If successful, calculates the actual light intensity in lux. Else, return -1.
    if (bytes_read == 2) {
        uint16_t raw = (buf[0] << 8) | buf[1];
        float lux = raw / 1.2f;
        reading->light_intensity = lux;
        return 0;
    } 
    
    return -1;
}