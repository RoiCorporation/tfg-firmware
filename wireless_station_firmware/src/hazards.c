#include <stdbool.h>
#include "wireless_station_firmware.h"
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
        is_pressure_rising = true, 
        is_air_quality_worsening = true;

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
        if (!is_pressure_rising || 
            (previous_readings[i].air_pressure <= previous_readings[i - 1].air_pressure) ||
            ((previous_readings[i].air_pressure - previous_readings[i - 1].air_pressure) < (PRESSURE_INCREASE_MARGIN - EPSILON)))
            is_pressure_rising = false;
        if (!is_air_quality_worsening || 
            (previous_readings[i].air_quality_index >= previous_readings[i - 1].air_quality_index) ||
            ((previous_readings[i].air_quality_index - previous_readings[i - 1].air_quality_index) > (AIR_QUALITY_WORSENING_MARGIN - EPSILON)))
            is_air_quality_worsening = false;
    }

    // If any of the conditions are worsening, return that hazard code. Else, return 0.
    if (is_temp_rising) return TEMPERATURE_RISING_HAZARD;
    if (is_humidity_rising) return HUMIDITY_RISING_HAZARD;
    //if (is_pressure_rising) return PRESSURE_RISING_HAZARD;
    //if (is_air_quality_worsening) return AIR_QUALITY_WORSENING_HAZARD;
    return 0;
}