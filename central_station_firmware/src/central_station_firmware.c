#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"

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

    while (true) {
        // Send
        char *msg = "Hello LoRa!\n";
        uart_puts(UART_ID, msg);

        // Read if available
        if (uart_is_readable(UART_ID)) {
            char c = uart_getc(UART_ID);
            printf("Received: %c\n", c);
        }

        sleep_ms(1000);
    }
}
