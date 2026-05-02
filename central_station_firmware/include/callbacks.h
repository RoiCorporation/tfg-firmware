#ifndef CALLBACKS_H
#define CALLBACKS_H


#include "pico/stdlib.h"


extern volatile button_action_t button_action;
extern volatile uint8_t display_turn_update;
extern absolute_time_t time_button_press;
extern absolute_time_t time_button_release;


void button_callback(uint gpio, uint32_t events);
bool display_turn_timer_callback(__unused struct repeating_timer *t);


#endif
