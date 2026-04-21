#include <stdbool.h>
#include "wireless_station_firmware.h"
#include "callbacks.h"



/**
 * @brief Handle the button press and release actions, updating the state
 * variable in charge of deciding which action the station must do next,
 * according to how long the button was pressed for.
 * 
 * @param gpio GPIO pin whose state update triggered the callback.
 * @param events which type of event triggered the callback.
 */
void button_callback(uint gpio, uint32_t events) {

    if (events & GPIO_IRQ_EDGE_RISE) {
        time_button_press = get_absolute_time();
        printf("Touched, time: %u ms\n", to_ms_since_boot(time_button_press));
    }
    else if (events & GPIO_IRQ_EDGE_FALL) {
        button_action = TURN_ON_DISPLAY;
        time_button_release = get_absolute_time();
        uint32_t elapsed_time_ms = (
            to_ms_since_boot(time_button_release) -
            to_ms_since_boot(time_button_press)
        );

        if (elapsed_time_ms >= BUTTON_PRESS_DELAY_FOR_HANDSHAKE_MS)
            button_action = START_HANDSHAKE;
        printf("Elapsed time: %u ms\n", elapsed_time_ms);
    }
    printf("\n");
}


/**
 * @brief Update the turn variables needed to control which environmental
 * measurement is shown in the OLED display and how many turns are left
 * before such display is set to sleep mode.
 * 
 * @param t pointer to a structure that stores the two state variables
 * related with the display turns.
 * @return true this method always returns true.
 */
bool display_turn_timer_callback(__unused struct repeating_timer *t) {
    display_timer_ctx_t *display_timer_ctx = (display_timer_ctx_t *)t->user_data;
    if (display_timer_ctx->display_turn == AMBIENT_INFO_FIELD_COUNT)
        display_timer_ctx->display_turn = 0;
    else
        (display_timer_ctx->display_turn)++;
    display_timer_ctx->turns_until_display_off--;
    return true;
}
