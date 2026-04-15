#include <stdbool.h>
#include "wireless_station_firmware.h"
#include "hazards.h"


/**
 * @brief Check if there are any potential hazards in the vicinity of the
 * station by analyzing the most relevant past few measurements.
 * 
 * @param previous_readings the list of past measurements taken by the sensors.
 * @return unsigned int 0 if no hazards were detected, else the numeric code that
 * represents that particular hazard (a positive integer).
 */
unsigned int analyze_hazards(ambient_info_t previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY]) {
    bool is_temp_rising = true,
        is_humidity_rising = true,
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
        if (!is_air_quality_worsening || 
            (previous_readings[i].air_quality_index <= previous_readings[i - 1].air_quality_index) ||
            ((previous_readings[i].air_quality_index - previous_readings[i - 1].air_quality_index) < (AIR_QUALITY_WORSENING_MARGIN - EPSILON)))
            is_air_quality_worsening = false;
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
    return 0;
}
