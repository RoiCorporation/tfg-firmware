#ifndef LOW_POWER_H
#define LOW_POWER_H


#include "wireless_station_firmware.h"
#include "hardware/powman.h"


/* GLOBAL VARIABLES */
extern powman_power_state off_state;
extern powman_power_state on_state;


void init_powman_states(void);
int8_t turn_low_power_on(void);


#endif
