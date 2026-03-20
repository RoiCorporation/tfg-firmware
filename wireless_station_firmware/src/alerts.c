#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "wireless_station_firmware.h"
#include "hazards.h"
#include "alerts.h"


/**
 * @brief Play an alert sound through the buzzer according to the potential 
 * hazard that the sensors have detected.
 * 
 * @param hazard_code the code that identifies the hazard type.
 */
void activate_hazard_alert(unsigned int hazard_code) {
    unsigned int slice = pwm_gpio_to_slice_num(BUZZER_PIN);
    unsigned int channel = pwm_gpio_to_channel(BUZZER_PIN);
    unsigned int frequency, bip_period = 100;
    
    switch(hazard_code) {
        case TEMPERATURE_RISING_HAZARD:
            play_temperature_hazard_alarm(slice, channel);
            break;
        case HUMIDITY_RISING_HAZARD:
            play_humidity_hazard_alarm(slice, channel);
            break;
        case PRESSURE_RISING_HAZARD:
            play_air_pressure_hazard_alarm(slice, channel);
            break;
        case AIR_QUALITY_WORSENING_HAZARD:
            play_air_quality_index_hazard_alarm(slice, channel);
            break;
        default:
            break;
    }
    
}


/**
 * @brief Use the buzzer to play the alarm specific to the rising temperature hazard.
 * 
 * @param slice the PWM slice number for the buzzer pin.
 * @param channel the PWM channel number for the buzzer pin.
 */
void play_temperature_hazard_alarm(unsigned int slice, unsigned int channel) {
    unsigned int frequencies[NUMBER_TONES_TEMPERATURE_HAZARD_ALARM] = {NOTE_B4, NOTE_F5};
    float clkdiv = 4.0f;

    for (int i = 0; i < NUMBER_ALARM_REPETITIONS; i++) {
        for (int t = 0; t < NUMBER_TONES_TEMPERATURE_HAZARD_ALARM; t++) {
            float wrap = (125000000 / (clkdiv * frequencies[t])) - 1;
            pwm_set_clkdiv(slice, clkdiv);
            pwm_set_wrap(slice, (unsigned int)wrap);
            pwm_set_chan_level(slice, channel, wrap / 2);

            pwm_set_enabled(slice, true);
            sleep_ms(TEMPERATURE_HAZARD_ALARM_TONE_DURATION);
        }
    }
    pwm_set_enabled(slice, false); // Turn off the buzzer after the alarm.
}


/**
 * @brief Use the buzzer to play the alarm specific to the rising temperature humidity.
 * 
 * @param slice the PWM slice number for the buzzer pin.
 * @param channel the PWM channel number for the buzzer pin.
 */
void play_humidity_hazard_alarm(unsigned int slice, unsigned int channel) {
    unsigned int frequencies[NUMBER_TONES_HUMIDITY_HAZARD_ALARM] = {NOTE_F5, NOTE_G5, NOTE_A5, NOTE_B5};
    float clkdiv = 4.0f;

    for (int i = 0; i < NUMBER_ALARM_REPETITIONS; i++) {
        for (int t = 0; t < NUMBER_TONES_HUMIDITY_HAZARD_ALARM; t++) {
            float wrap = (125000000 / (clkdiv * frequencies[t])) - 1;
            pwm_set_clkdiv(slice, clkdiv);
            pwm_set_wrap(slice, (unsigned int)wrap);
            pwm_set_chan_level(slice, channel, wrap / 2);

            pwm_set_enabled(slice, true);
            sleep_ms(HUMIDITY_HAZARD_ALARM_TONE_DURATION);
        }
    }
    pwm_set_enabled(slice, false); // Turn off the buzzer after the alarm.
}


/**
 * @brief Use the buzzer to play the alarm specific to the rising air pressure hazard.
 * 
 * @param slice the PWM slice number for the buzzer pin.
 * @param channel the PWM channel number for the buzzer pin.
 */
void play_air_pressure_hazard_alarm(unsigned int slice, unsigned int channel) {
    unsigned int frequencies[2] = {NOTE_B4, NOTE_F5};
    float clkdiv = 4.0f;

    for (int i = 0; i < NUMBER_ALARM_REPETITIONS; i++) {
        for (int t = 0; t < 2; t++) {
            float wrap = (125000000 / (clkdiv * frequencies[t])) - 1;
            pwm_set_clkdiv(slice, clkdiv);
            pwm_set_wrap(slice, (unsigned int)wrap);
            pwm_set_chan_level(slice, channel, wrap / 2);

            pwm_set_enabled(slice, true);
            sleep_ms(PRESSURE_HAZARD_ALARM_TONE_DURATION);
        }
    }
    pwm_set_enabled(slice, false); // Turn off the buzzer after the alarm.
}


/**
 * @brief Use the buzzer to play the alarm specific to the worsening AQI hazard.
 * 
 * @param slice the PWM slice number for the buzzer pin.
 * @param channel the PWM channel number for the buzzer pin.
 */
void play_air_quality_index_hazard_alarm(unsigned int slice, unsigned int channel) {
    unsigned int frequencies[2] = {NOTE_B4, NOTE_F5};
    float clkdiv = 4.0f;

    for (int i = 0; i < NUMBER_ALARM_REPETITIONS; i++) {
        for (int t = 0; t < 2; t++) {
            float wrap = (125000000 / (clkdiv * frequencies[t])) - 1;
            pwm_set_clkdiv(slice, clkdiv);
            pwm_set_wrap(slice, (unsigned int)wrap);
            pwm_set_chan_level(slice, channel, wrap / 2);

            pwm_set_enabled(slice, true);
            sleep_ms(AIR_QUALITY_INDEX_HAZARD_ALARM_TONE_DURATION);
        }
    }
    pwm_set_enabled(slice, false); // Turn off the buzzer after the alarm.
}