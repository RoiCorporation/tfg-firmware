#ifndef HAZARDS_H
#define HAZARDS_H


#include "wireless_station_firmware.h"


// === Identifiers for the different hazards ===
#define TEMPERATURE_RISING_HAZARD 0x31
#define HUMIDITY_RISING_HAZARD 0x32
#define AIR_QUALITY_WORSENING_HAZARD 0x33


// === Function declarations ===
unsigned int analyze_hazards(ambient_info_t previous_readings[]);


#endif
