#ifndef CALLBACKS_H
#define CALLBACKS_H


#include "pico/stdlib.h"
#include "wireless_station_firmware.h"


/* GLOBAL VARIABLES */
extern absolute_time_t time_button_press;
extern absolute_time_t time_button_release;
extern struct repeating_timer display_turn_change_timer;
extern display_timer_ctx_t display_timer_ctx;
extern volatile button_action_t button_action;


void button_callback(uint gpio, uint32_t events);
bool display_turn_timer_callback(__unused struct repeating_timer *t);


#endif
