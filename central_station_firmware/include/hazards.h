#ifndef HAZARDS_H
#define HAZARDS_H


#include "central_station_firmware.h"


// === Identifiers for the different hazards ===
#define TEMPERATURE_RISING_HAZARD 0x31
#define HUMIDITY_RISING_HAZARD 0x32
#define AIR_QUALITY_WORSENING_HAZARD 0x33
#define CARBON_MONOXIDE_WORSENING_HAZARD 0x34
#define METHANE_WORSENING_HAZARD 0x35
#define PROPANE_WORSENING_HAZARD 0x36
#define HYDROGEN_GAS_WORSENING_HAZARD 0x37


// === Function declarations ===
unsigned int analyze_hazards(ambient_info_t previous_readings[]);


#endif
