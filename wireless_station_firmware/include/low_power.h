#ifndef LOW_POWER_H
#define LOW_POWER_H


#include "wireless_station_firmware.h"
#include "hardware/powman.h"


/* GLOBAL VARIABLES */
extern retained_data_t *data_retained_in_hibernation;


void init_powman_states(void);
void turn_low_power_on(void);
void save_retained_data_to_flash();


#endif
