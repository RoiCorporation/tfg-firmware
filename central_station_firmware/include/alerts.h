#ifndef ALERTS_H
#define ALERTS_H


// === Musical note frequencies ===
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_B5  988


// === Constants related to the alarms ===
#define NUMBER_ALARM_REPETITIONS 10

// Number of different tones in each hazard alarm.
#define NUMBER_TONES_TEMPERATURE_HAZARD_ALARM 2
#define NUMBER_TONES_HUMIDITY_HAZARD_ALARM 4
#define NUMBER_TONES_AIR_QUALITY_INDEX_HAZARD_ALARM 3
#define NUMBER_TONES_CARBON_MONOXIDE_HAZARD_ALARM 2
#define NUMBER_TONES_METHANE_HAZARD_ALARM 5
#define NUMBER_TONES_PROPANE_HAZARD_ALARM 3
#define NUMBER_TONES_HYDROGEN_GAS_HAZARD_ALARM 3

// Duration of each tone in the hazard alarms (in milliseconds).
#define TEMPERATURE_HAZARD_ALARM_TONE_DURATION 600
#define HUMIDITY_HAZARD_ALARM_TONE_DURATION 200
#define AIR_QUALITY_INDEX_HAZARD_ALARM_TONE_DURATION 400
#define CARBON_MONOXIDE_HAZARD_ALARM_TONE_DURATION 150
#define METHANE_HAZARD_ALARM_TONE_DURATION 700
#define PROPANE_HAZARD_ALARM_TONE_DURATION 300
#define HYDROGEN_GAS_HAZARD_ALARM_TONE_DURATION 350


// === Function declarations ===
void activate_hazard_alert(unsigned int hazard_code);
void play_temperature_hazard_alarm(unsigned int slice, unsigned int channel);
void play_humidity_hazard_alarm(unsigned int slice, unsigned int channel);
void play_air_quality_index_hazard_alarm(unsigned int slice, unsigned int channel);
void play_carbon_monoxide_hazard_alarm(unsigned int slice, unsigned int channel);
void play_methane_hazard_alarm(unsigned int slice, unsigned int channel);
void play_propane_hazard_alarm(unsigned int slice, unsigned int channel);
void play_hydrogen_gas_hazard_alarm(unsigned int slice, unsigned int channel);


#endif
