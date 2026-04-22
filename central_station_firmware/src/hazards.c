#include <stdbool.h>
#include "central_station_firmware.h"
#include "hazards.h"


/**
 * @brief Check if there are any potential hazards in the vicinity of the
 * station by analyzing the past few measurements taken by all sensors.
 * 
 * @param previous_readings the list of past measurements taken by the sensors.
 * @return unsigned int 0 if no hazards were detected, else the numeric code that
 * represents that particular hazard (a positive integer).
 */
unsigned int analyze_hazards(ambient_info_t previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY]) {
    bool is_temp_rising = true,
        is_humidity_rising = true,
        is_air_quality_worsening = true,
        is_carbon_monoxide_worsening = true,
        is_methane_worsening = true,
        is_propane_worsening = true,
        is_alcohol_worsening = true,
        is_hydrogen_gas_worsening = true;

    // Loop through the array and check if any of the ambient conditions is worsening.
    for (int i = 1; i < LENGTH_PREVIOUS_READINGS_ARRAY; i++) {
        if (!is_temp_rising || 
            (previous_readings[i].temperature <= previous_readings[i - 1].temperature) || 
            ((previous_readings[i].temperature - previous_readings[i - 1].temperature) < (TEMPERATURE_INCREASE_MARGIN - EPSILON)))
            is_temp_rising = false;
        if (!is_humidity_rising || 
            (previous_readings[i].humidity <= previous_readings[i - 1].humidity) ||
            ((previous_readings[i].humidity - previous_readings[i - 1].humidity) < (HUMIDITY_INCREASE_MARGIN - EPSILON)))
            is_humidity_rising = false;
        if (!is_air_quality_worsening || 
            (previous_readings[i].air_quality_index <= previous_readings[i - 1].air_quality_index) ||
            ((previous_readings[i].air_quality_index - previous_readings[i - 1].air_quality_index) < (AIR_QUALITY_WORSENING_MARGIN - EPSILON)))
            is_air_quality_worsening = false;
        if (!is_carbon_monoxide_worsening ||
            (previous_readings[i].carbon_monoxide_concentration <= previous_readings[i - 1].carbon_monoxide_concentration) ||
            ((previous_readings[i].carbon_monoxide_concentration - previous_readings[i - 1].carbon_monoxide_concentration) < 
            (CARBON_MONOXIDE_WORSENING_MARGIN - EPSILON)))
            is_carbon_monoxide_worsening = false;
        if (!is_methane_worsening || 
            (previous_readings[i].methane_concentration <= previous_readings[i - 1].methane_concentration) ||
            ((previous_readings[i].methane_concentration - previous_readings[i - 1].methane_concentration) 
            < (METHANE_WORSENING_MARGIN - EPSILON)))
            is_methane_worsening = false;
        if (!is_propane_worsening || 
            (previous_readings[i].propane_concentration <= previous_readings[i - 1].propane_concentration) ||
            ((previous_readings[i].propane_concentration - previous_readings[i - 1].propane_concentration) 
            < (PROPANE_WORSENING_MARGIN - EPSILON)))
            is_propane_worsening = false;
        if (!is_alcohol_worsening || 
            (previous_readings[i].alcohol_concentration <= previous_readings[i - 1].alcohol_concentration) ||
            ((previous_readings[i].alcohol_concentration - previous_readings[i - 1].alcohol_concentration) 
            < (ALCOHOL_WORSENING_MARGIN - EPSILON)))
            is_alcohol_worsening = false;
        if (!is_hydrogen_gas_worsening || 
            (previous_readings[i].hydrogen_gas_concentration <= previous_readings[i - 1].hydrogen_gas_concentration) ||
            ((previous_readings[i].hydrogen_gas_concentration - previous_readings[i - 1].hydrogen_gas_concentration)
            < (HYDROGEN_GAS_WORSENING_MARGIN - EPSILON)))
            is_hydrogen_gas_worsening = false;
    }

    // If any of the conditions are worsening or exceed the threshold,
    // return that hazard code. Otherwise return 0.
    if (is_temp_rising || 
        previous_readings[0].temperature > TEMPERATURE_HAZARD_THRESHOLD ||
        previous_readings[1].temperature > TEMPERATURE_HAZARD_THRESHOLD ||
        previous_readings[2].temperature > TEMPERATURE_HAZARD_THRESHOLD ||
        previous_readings[3].temperature > TEMPERATURE_HAZARD_THRESHOLD ||
        previous_readings[4].temperature > TEMPERATURE_HAZARD_THRESHOLD
    ) return TEMPERATURE_RISING_HAZARD;
    if (is_humidity_rising) return HUMIDITY_RISING_HAZARD;
    if (is_air_quality_worsening ||
        previous_readings[0].air_quality_index > AIR_QUALITY_HAZARD_THRESHOLD ||
        previous_readings[1].air_quality_index > AIR_QUALITY_HAZARD_THRESHOLD ||
        previous_readings[2].air_quality_index > AIR_QUALITY_HAZARD_THRESHOLD ||
        previous_readings[3].air_quality_index > AIR_QUALITY_HAZARD_THRESHOLD ||
        previous_readings[4].air_quality_index > AIR_QUALITY_HAZARD_THRESHOLD
    ) return AIR_QUALITY_WORSENING_HAZARD;
    if (is_carbon_monoxide_worsening || 
        previous_readings[0].carbon_monoxide_concentration > CARBON_MONOXIDE_HAZARD_THRESHOLD ||
        previous_readings[1].carbon_monoxide_concentration > CARBON_MONOXIDE_HAZARD_THRESHOLD ||
        previous_readings[2].carbon_monoxide_concentration > CARBON_MONOXIDE_HAZARD_THRESHOLD ||
        previous_readings[3].carbon_monoxide_concentration > CARBON_MONOXIDE_HAZARD_THRESHOLD ||
        previous_readings[4].carbon_monoxide_concentration > CARBON_MONOXIDE_HAZARD_THRESHOLD
    ) return CARBON_MONOXIDE_WORSENING_HAZARD;
    if (is_methane_worsening || 
        previous_readings[0].methane_concentration > METHANE_HAZARD_THRESHOLD ||
        previous_readings[1].methane_concentration > METHANE_HAZARD_THRESHOLD ||
        previous_readings[2].methane_concentration > METHANE_HAZARD_THRESHOLD ||
        previous_readings[3].methane_concentration > METHANE_HAZARD_THRESHOLD ||
        previous_readings[4].methane_concentration > METHANE_HAZARD_THRESHOLD
    ) return METHANE_WORSENING_HAZARD;
    if (is_propane_worsening || 
        previous_readings[0].propane_concentration > PROPANE_HAZARD_THRESHOLD ||
        previous_readings[1].propane_concentration > PROPANE_HAZARD_THRESHOLD ||
        previous_readings[2].propane_concentration > PROPANE_HAZARD_THRESHOLD ||
        previous_readings[3].propane_concentration > PROPANE_HAZARD_THRESHOLD ||
        previous_readings[4].propane_concentration > PROPANE_HAZARD_THRESHOLD
    ) return PROPANE_WORSENING_HAZARD;
    if (is_alcohol_worsening || 
        previous_readings[0].alcohol_concentration > ALCOHOL_HAZARD_THRESHOLD ||
        previous_readings[1].alcohol_concentration > ALCOHOL_HAZARD_THRESHOLD ||
        previous_readings[2].alcohol_concentration > ALCOHOL_HAZARD_THRESHOLD ||
        previous_readings[3].alcohol_concentration > ALCOHOL_HAZARD_THRESHOLD ||
        previous_readings[4].alcohol_concentration > ALCOHOL_HAZARD_THRESHOLD
    ) return ALCOHOL_WORSENING_HAZARD;
    if (is_hydrogen_gas_worsening || 
        previous_readings[0].hydrogen_gas_concentration > HYDROGEN_GAS_HAZARD_THRESHOLD ||
        previous_readings[1].hydrogen_gas_concentration > HYDROGEN_GAS_HAZARD_THRESHOLD ||
        previous_readings[2].hydrogen_gas_concentration > HYDROGEN_GAS_HAZARD_THRESHOLD ||
        previous_readings[3].hydrogen_gas_concentration > HYDROGEN_GAS_HAZARD_THRESHOLD ||
        previous_readings[4].hydrogen_gas_concentration > HYDROGEN_GAS_HAZARD_THRESHOLD
    ) return HYDROGEN_GAS_WORSENING_HAZARD;
    return 0;
}
