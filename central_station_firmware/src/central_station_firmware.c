#include "stdio.h"
#include "pico/stdlib.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "nrf24_driver.h"
#include "central_station_firmware.h"

#define UART_ID uart1
#define BAUD_RATE 9600

#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define M0_PIN 2
#define M1_PIN 3
#define AUX_PIN 16

int main() {
    stdio_init_all();

    // Configure mode pins
    gpio_init(M0_PIN);
    gpio_init(M1_PIN);
    gpio_set_dir(M0_PIN, GPIO_OUT);
    gpio_set_dir(M1_PIN, GPIO_OUT);
    gpio_put(M0_PIN, 0);
    gpio_put(M1_PIN, 0);  // Normal mode (M1=0, M0=0)

    // UART setup
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(UART_ID, true);


    pin_manager_t my_pins = { .sck = SCK_PIN, .copi = COPI_PIN, .cipo = CIPO_PIN, .csn = CS_PIN, .ce = CE_PIN };

    nrf_manager_t my_config = {
        // RF Channel 
        .channel = 120,

        // AW_3_BYTES, AW_4_BYTES, AW_5_BYTES
        .address_width = AW_5_BYTES,

        // dynamic payloads: DYNPD_ENABLE, DYNPD_DISABLE
        .dyn_payloads = DYNPD_ENABLE,

        // data rate: RF_DR_250KBPS, RF_DR_1MBPS, RF_DR_2MBPS
        .data_rate = RF_DR_1MBPS,

        // RF_PWR_NEG_18DBM, RF_PWR_NEG_12DBM, RF_PWR_NEG_6DBM, RF_PWR_0DBM
        .power = RF_PWR_NEG_12DBM,

        // retransmission count: ARC_NONE...ARC_15RT
        .retr_count = ARC_10RT,

        // retransmission delay: ARD_250US, ARD_500US, ARD_750US, ARD_1000US
        .retr_delay = ARD_500US 
    };

    // SPI baudrate
    uint32_t my_baudrate = 5000000;

    // provides access to driver functions
    nrf_client_t my_nrf;

    // initialise my_nrf
    nrf_driver_create_client(&my_nrf);

    // configure GPIO pins and SPI
    my_nrf.configure(&my_pins, my_baudrate);

    // not using default configuration (my_nrf.initialise(NULL)) 
    my_nrf.initialise(&my_config);

    /**
     * set addresses for DATA_PIPE_0 - DATA_PIPE_3.
     * These are addresses the transmitter will send its packets to.
     */
    my_nrf.rx_destination(DATA_PIPE_0, (uint8_t[]){0x37,0x37,0x37,0x37,0x37});
    my_nrf.rx_destination(DATA_PIPE_1, (uint8_t[]){0xC7,0xC7,0xC7,0xC7,0xC7});
    my_nrf.rx_destination(DATA_PIPE_2, (uint8_t[]){0xC8,0xC7,0xC7,0xC7,0xC7});
    my_nrf.rx_destination(DATA_PIPE_3, (uint8_t[]){0xC9,0xC7,0xC7,0xC7,0xC7});

    // set to RX Mode
    my_nrf.receiver_mode();

    // data pipe number a packet was received on
    uint8_t pipe_number = 0;

    // holds payload_zero sent by the transmitter
    uint8_t payload_zero = 0;

    // holds payload_one sent by the transmitter
    uint8_t payload_one[5];

    // two byte struct sent by transmitter
    typedef struct payload_two_s { uint8_t one; uint8_t two; } payload_two_t;

    // holds payload_two struct sent by the transmitter
    payload_two_t payload_two;

    while (1)
    {
        if (my_nrf.is_packet(&pipe_number)) {
            switch (pipe_number) {
                case DATA_PIPE_0:
                // read payload
                my_nrf.read_packet(&payload_zero, sizeof(payload_zero));

                // receiving a one byte uint8_t payload on DATA_PIPE_0
                printf("\nPacket received:- Payload (%d) on data pipe (%d)\n", payload_zero, pipe_number);
                break;
                
                case DATA_PIPE_1:
                // read payload
                my_nrf.read_packet(payload_one, sizeof(payload_one));

                // receiving a five byte string payload on DATA_PIPE_1
                printf("\nPacket received:- Payload (%s) on data pipe (%d)\n", payload_one, pipe_number);
                break;
                
                case DATA_PIPE_2:
                // read payload
                my_nrf.read_packet(&payload_two, sizeof(payload_two));

                // receiving a two byte struct payload on DATA_PIPE_2
                printf("\nPacket received:- Payload (1: %d, 2: %d) on data pipe (%d)\n", payload_two.one, payload_two.two, pipe_number);
                break;
                
                case DATA_PIPE_3:
                break;
                
                case DATA_PIPE_4:
                break;
                
                case DATA_PIPE_5:
                break;
                
                default:
                break;
            }
        }
    }
}
