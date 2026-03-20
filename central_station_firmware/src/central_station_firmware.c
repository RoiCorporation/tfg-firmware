#include "stdio.h"
#include "pico/stdlib.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "nrf24_driver.h"
#include "central_station_firmware.h"


int main() {

    initialize_board();

    pin_manager_t nrf24_pins = {.sck = SCK_PIN, .copi = COPI_PIN, .cipo = CIPO_PIN, .csn = CS_PIN, .ce = CE_PIN};

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
        .retr_delay = ARD_500US};

    // SPI baudrate
    uint32_t my_baudrate = 7000000;

    // provides access to driver functions
    nrf_client_t nrf24_module;

    // initialise my_nrf
    nrf_driver_create_client(&nrf24_module);

    // configure GPIO pins and SPI
    nrf24_module.configure(&nrf24_pins, my_baudrate);

    while (!stdio_usb_connected()) sleep_ms(10);

    // not using default configuration (my_nrf.initialise(NULL))
    nrf24_module.initialise(&nrf24_config);
    // nrf24_module.initialise(NULL);
    // nrf24_module.dyn_payloads_enable();

    /**
     * set addresses for DATA_PIPE_0 - DATA_PIPE_3.
     * These are addresses the transmitter will send its packets to.
     */
    nrf24_module.rx_destination(DATA_PIPE_1, (uint8_t[]){0xC7, 0xC7, 0xC7, 0xC7, 0xC7});

    // set to RX Mode
    nrf24_module.receiver_mode();

    // holds payload_one sent by the transmitter
    int payload_array_size = sizeof(ambient_info_t) / sizeof(float);
    float payload[payload_array_size];

    while (1) {
        if (nrf24_module.is_packet(NULL)) {
            if (nrf24_module.read_packet(&payload, sizeof(payload)) == 0) {
                printf("Error receiving\n");
            }
            else {
                printf("Packet received\n");
                for (int i = 0; i < payload_array_size; i++) {
                    printf("%d: %f\n", i, payload[i]);
                }
            }
        }
    }

}
