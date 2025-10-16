#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "wireless_station_firmware.h"
#include "errors.h"


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
            ((previous_readings[i].temperature - previous_readings[i - 1].temperature) < TEMPERATURE_INCREASE_MARGIN))
            is_temp_rising = false;
        if (!is_humidity_rising || 
            (previous_readings[i].humidity <= previous_readings[i - 1].humidity) ||
            ((previous_readings[i].humidity - previous_readings[i - 1].humidity) < HUMIDITY_INCREASE_MARGIN))
            is_humidity_rising = false;
        if (!is_pressure_rising || 
            (previous_readings[i].pressure <= previous_readings[i - 1].pressure) ||
            ((previous_readings[i].pressure - previous_readings[i - 1].pressure) < PRESSURE_INCREASE_MARGIN))
            is_pressure_rising = false;
        if (!is_air_quality_worsening || 
            (previous_readings[i].air_quality_index >= previous_readings[i - 1].air_quality_index) ||
            ((previous_readings[i].air_quality_index - previous_readings[i - 1].air_quality_index) > AIR_QUALITY_WORSENING_MARGIN))
            is_air_quality_worsening = false;
    }

    // If any of the conditions are worsening, return that hazard code. Else, return 0.
    if (is_temp_rising) return TEMPERATURE_RISING_HAZARD;
    if (is_humidity_rising) return HUMIDITY_RISING_HAZARD;
    //if (is_pressure_rising) return PRESSURE_RISING_HAZARD;
    //if (is_air_quality_worsening) return AIR_QUALITY_WORSENING_HAZARD;
    return 0;
}


/**
 * @brief Play an alert sound through the buzzer according to the potential 
 * hazard that the sensors have detected.
 * 
 * @param hazard_code the code that identifies the hazard type.
 */
void play_hazard_alert(unsigned int hazard_code) {
    uint slice = pwm_gpio_to_slice_num(BUZZER_PIN);
    uint channel = pwm_gpio_to_channel(BUZZER_PIN);
    uint frequency, bip_period;
    
    switch(hazard_code) {
        case TEMPERATURE_RISING_HAZARD:
            frequency = 900;
            bip_period = 100;
            break;
        case HUMIDITY_RISING_HAZARD:
            frequency = 520;
            bip_period = 500;
            break;
        case PRESSURE_RISING_HAZARD:
            frequency = 800;
            bip_period = 2000;
            break;
        case AIR_QUALITY_WORSENING_HAZARD:
            frequency = 700;
            bip_period = 4000;
            break;
        default:
            break;
    }

    // Calculate wrap for the desired frequency.
    float clkdiv = 4.0f;
    float wrap = (125000000 / (clkdiv * frequency)) - 1;

    pwm_set_clkdiv(slice, clkdiv);
    pwm_set_wrap(slice, (uint)wrap);
    pwm_set_chan_level(slice, channel, wrap / 2); // Set a 50% duty cycle.
    pwm_set_enabled(slice, true);

    sleep_ms(bip_period);
    pwm_set_enabled(slice, false);
}