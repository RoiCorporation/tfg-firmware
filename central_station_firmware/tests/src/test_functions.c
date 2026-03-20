#include "central_station_firmware.h"
#include "errors.h"


/**
 * @brief Reset the previous readings list by setting all the elements to zero.
 * 
 * @param previous_readings the previous readings list to reset.
 */
void clean_previous_readings_list(ambient_info_t previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY]) {
    for (int i = 0; i < LENGTH_PREVIOUS_READINGS_ARRAY; i++) {
        previous_readings[i].temperature = 0.0;
        previous_readings[i].humidity = 0.0;
        previous_readings[i].air_pressure = 0.0;
        previous_readings[i].air_quality_index = 0.0;
        previous_readings[i].light_intensity = 0.0;
    }
}


/**
 * @brief Construct a list of prior readings that presents a hazard due to increasing temperature.
 * 
 * @param previous_readings the previous readings list where the test data will be stored.
 */
void create_rising_temperature_hazard(ambient_info_t previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY]) {
  ambient_info_t initial_values = {-2.4, 13.7, 3.0, 7.5, 267.77};
  previous_readings[0] = initial_values;
  for (int i = 1; i < LENGTH_PREVIOUS_READINGS_ARRAY; i++) {
    previous_readings[i].temperature = previous_readings[i - 1].temperature + (TEMPERATURE_INCREASE_MARGIN);
    previous_readings[i].humidity = previous_readings[i - 1].humidity + (HUMIDITY_INCREASE_MARGIN - 0.05);
    previous_readings[i].air_pressure = previous_readings[i - 1].air_pressure + (PRESSURE_INCREASE_MARGIN - 0.05);
    previous_readings[i].air_quality_index = previous_readings[i - 1].air_quality_index + (AIR_QUALITY_WORSENING_MARGIN - 0.05);
  }
}


/**
 * @brief Construct a list of prior readings that presents a hazard due to increasing humidity.
 * 
 * @param previous_readings the previous readings list where the test data will be stored.
 */
void create_rising_humidity_hazard(ambient_info_t previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY]) {
  ambient_info_t initial_values = {-2.4, 13.7, 3.0, 7.5, 267.77};
  previous_readings[0] = initial_values;
  for (int i = 1; i < LENGTH_PREVIOUS_READINGS_ARRAY; i++) {
    previous_readings[i].temperature = previous_readings[i - 1].temperature + (TEMPERATURE_INCREASE_MARGIN - 0.05);
    previous_readings[i].humidity = previous_readings[i - 1].humidity + (HUMIDITY_INCREASE_MARGIN);
    previous_readings[i].air_pressure = previous_readings[i - 1].air_pressure + (PRESSURE_INCREASE_MARGIN - 0.05);
    previous_readings[i].air_quality_index = previous_readings[i - 1].air_quality_index + (AIR_QUALITY_WORSENING_MARGIN - 0.05);
  }
}


/**
 * @brief Construct a list of prior readings that presents a hazard due to increasing air air_pressure.
 * 
 * @param previous_readings the previous readings list where the test data will be stored.
 */
void create_rising_pressure_hazard(ambient_info_t previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY]) {
  ambient_info_t initial_values = {-2.4, 13.7, 3.0, 7.5, 267.77};
  previous_readings[0] = initial_values;
  for (int i = 1; i < LENGTH_PREVIOUS_READINGS_ARRAY; i++) {
    previous_readings[i].temperature = previous_readings[i - 1].temperature + (TEMPERATURE_INCREASE_MARGIN - 0.05);
    previous_readings[i].humidity = previous_readings[i - 1].humidity + (HUMIDITY_INCREASE_MARGIN - 0.05);
    previous_readings[i].air_pressure = previous_readings[i - 1].air_pressure + (PRESSURE_INCREASE_MARGIN);
    previous_readings[i].air_quality_index = previous_readings[i - 1].air_quality_index + (AIR_QUALITY_WORSENING_MARGIN - 0.05);
  }
}


/**
 * @brief Construct a list of prior readings that presents a hazard due to air quality 
 * continuously worsening.
 * 
 * @param previous_readings the previous readings list where the test data will be stored.
 */
void create_worsening_air_quality_hazard(ambient_info_t previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY]) {
  ambient_info_t initial_values = {-2.4, 13.7, 3.0, 7.5, 267.77};
  previous_readings[0] = initial_values;
  for (int i = 1; i < LENGTH_PREVIOUS_READINGS_ARRAY; i++) {
    previous_readings[i].temperature = previous_readings[i - 1].temperature + (TEMPERATURE_INCREASE_MARGIN - 0.05);
    previous_readings[i].humidity = previous_readings[i - 1].humidity + (HUMIDITY_INCREASE_MARGIN - 0.05);
    previous_readings[i].air_pressure = previous_readings[i - 1].air_pressure + (PRESSURE_INCREASE_MARGIN - 0.05);
    previous_readings[i].air_quality_index = previous_readings[i - 1].air_quality_index + (AIR_QUALITY_WORSENING_MARGIN);
  }
}


/**
 * @brief Construct a list of prior readings that doesn't present any hazards.
 * 
 * @param previous_readings the previous readings list where the test data will be stored.
 */
void create_correct_previous_readings_list(ambient_info_t previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY]) {
  ambient_info_t initial_values = {-2.4, 13.7, 3.0, 7.5, 267.77};
  previous_readings[0] = initial_values;
  for (int i = 1; i < LENGTH_PREVIOUS_READINGS_ARRAY; i++) {
    previous_readings[i].temperature = previous_readings[i - 1].temperature + (TEMPERATURE_INCREASE_MARGIN - 0.05);
    previous_readings[i].humidity = previous_readings[i - 1].humidity + (HUMIDITY_INCREASE_MARGIN - 0.05);
    previous_readings[i].air_pressure = previous_readings[i - 1].air_pressure + (PRESSURE_INCREASE_MARGIN - 0.05);
    previous_readings[i].air_quality_index = previous_readings[i - 1].air_quality_index + (AIR_QUALITY_WORSENING_MARGIN - 0.05);
  }
}