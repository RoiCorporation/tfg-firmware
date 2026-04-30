#include <stdio.h>
#include <stdbool.h>
#include "wireless_station_firmware.h"
#include "callbacks.h"
#include "errors.h"
#include "oled_display.h"


bool minute_timer_callback(__unused struct repeating_timer *t) {
    minute_task_pending = 1;
    return true;
}


/**
 * @brief Handle the button press and release actions. When the button is
 * released, calculate whether it was a short or a long press and activate
 * the OLED display and start the measurement-displaying sequence. If it was
 * a long press, invoke the handshake protocol as well.
 * 
 * @param gpio GPIO pin whose state update triggered the callback.
 * @param events which type of event triggered the callback (
 * GPIO_IRQ_EDGE_RISE when pressing the button and GPIO_IRQ_EDGE_FALL when
 * releasing it).
 */
void button_callback(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_RISE) {
        time_button_press = get_absolute_time();
    }
    else if (events & GPIO_IRQ_EDGE_FALL) {
        button_action = TURN_ON_DISPLAY;
        time_button_release = get_absolute_time();
        uint32_t elapsed_time_ms = (
            to_ms_since_boot(time_button_release) -
            to_ms_since_boot(time_button_press)
        );

        // If it was a long press, invoke the handshake protocol to associate this
        // wireless station with a central one.
        if (elapsed_time_ms >= BUTTON_PRESS_DELAY_FOR_HANDSHAKE_MS) {
            button_action = START_HANDSHAKE;

        //     // Start the handshake procedure.
        //     int8_t handshake_result = handshake(
        //         button_ctx.nrf24_module,
        //         button_ctx.ecdh_private_key,
        //         button_ctx.ecdh_public_key,
        //         button_ctx.kdf_salt,
        //         button_ctx.aes_ctx,
        //         button_ctx.aes_key,
        //         button_ctx.aes_iv
        //     );

        //     // If the handshake procedure failed, print an appropriate error message.
        //     if (handshake_result != 0) {
        //         printf("Error on the handshake when associating a wireless station. Error number %d\n",
        //             HANDSHAKE_ERROR);
        //         ssd1306_draw_string(button_ctx.oled_display, 0, 16, 1, "An error occurred");
        //         ssd1306_draw_string(button_ctx.oled_display, 0, 32, 1, "trying to connect to");
        //         ssd1306_draw_string(button_ctx.oled_display, 0, 48, 1, "a central station.");
        //         ssd1306_show(button_ctx.oled_display);
        //     }
        }
        button_event_pending = 1;
    }
}


/**
 * @brief Update the OLED display with the environmental measurement that
 * corresponds to the current turn. Also update the state variables needed
 * to control both the turn system and the amount of turns before the display
 * is put into sleep mode.
 * 
 * @param t unused pointer.
 * @return true this method always returns true.
 */
bool display_turn_timer_callback(__unused struct repeating_timer *t) {

    switch(display_timer_ctx.display_turn) {
        case 1:
            display_temperature(
                display_timer_ctx.oled_display,
                display_timer_ctx.station_readings->temperature
            );
            break;
        case 2:
            display_humidity(
                display_timer_ctx.oled_display,
                display_timer_ctx.station_readings->humidity
            );
            break;
        case 3:
            display_light_intensity(
                display_timer_ctx.oled_display,
                display_timer_ctx.station_readings->light_intensity
            );
            break;
        case 4:
            display_air_pressure(
                display_timer_ctx.oled_display,
                display_timer_ctx.station_readings->air_pressure
            );
            break;
        case 5:
            display_air_quality_index(
                display_timer_ctx.oled_display,
                display_timer_ctx.station_readings->air_quality_index
            );
            break;
        default:
            break;
    }

    if (display_timer_ctx.display_turn == AMBIENT_INFO_FIELD_COUNT)
        display_timer_ctx.display_turn = 1;
    else
        (display_timer_ctx.display_turn)++;

    if (display_timer_ctx.turns_until_display_off > 0)
        display_timer_ctx.turns_until_display_off--;
    else {
        cancel_repeating_timer(&display_turn_change_timer);
        ssd1306_clear(display_timer_ctx.oled_display);
        ssd1306_show(display_timer_ctx.oled_display);
        ssd1306_poweroff(display_timer_ctx.oled_display);
    }

    return true;
}
