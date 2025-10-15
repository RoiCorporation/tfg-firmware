#include "unity.h"
#include "wireless_station_firmware.h"
#include "errors.h"


TEST_SOURCE_FILE("sensor_utils.c")

void setUp(void) {}
void tearDown(void) {}

void create_rising_temperature_hazard(ambient_info_t previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY]);
void create_rising_humidity_hazard(ambient_info_t previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY]);
void create_rising_pressure_hazard(ambient_info_t previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY]);
void create_worsening_air_quality_hazard(ambient_info_t previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY]);
void create_correct_previous_readings_list(ambient_info_t previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY]);


void test_analyze_hazards(void) {
  ambient_info_t previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY];
  
  // Test with a list of previous readings that poses a hazard due to
  // temperature rising continuously above the established safeguard.
  create_rising_temperature_hazard(previous_readings);
  TEST_ASSERT_EQUAL_INT(TEMPERATURE_RISING_HAZARD, analyze_hazards(previous_readings));

  // Test with a list of previous readings that poses a hazard due to
  // humidity rising continuously above the established safeguard.
  create_rising_humidity_hazard(previous_readings);
  TEST_ASSERT_EQUAL_INT(HUMIDITY_RISING_HAZARD, analyze_hazards(previous_readings));

  // TODO: Uncomment these last two cases when the firmware is ready to 
  // read the pressure and air quality data.
  // Test with a list of previous readings that poses a hazard due to
  // air pressure rising continuously above the established safeguard.
  // create_rising_pressure_hazard(previous_readings);
  // TEST_ASSERT_EQUAL_INT(PRESSURE_RISING_HAZARD, analyze_hazards(previous_readings));

  // Test with a list of previous readings that poses a hazard due to
  // air quality worsening continuously beyond the established safeguard.
  // create_worsening_air_quality_hazard(previous_readings);
  // TEST_ASSERT_EQUAL_INT(AIR_QUALITY_WORSENING_HAZARD, analyze_hazards(previous_readings));

  // Test with a list of previous readings that doesn't present any hazards whatsoever.
  create_correct_previous_readings_list(previous_readings);
  TEST_ASSERT_EQUAL_INT(0, analyze_hazards(previous_readings));
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
    previous_readings[i].pressure = previous_readings[i - 1].pressure + (PRESSURE_INCREASE_MARGIN - 0.05);
    previous_readings[i].air_quality_index = previous_readings[i - 1].air_quality_index + (AIR_QUALITY_WORSENING_MARGIN - 0.05);
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
    previous_readings[i].pressure = previous_readings[i - 1].pressure + (PRESSURE_INCREASE_MARGIN - 0.05);
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
    previous_readings[i].pressure = previous_readings[i - 1].pressure + (PRESSURE_INCREASE_MARGIN - 0.05);
    previous_readings[i].air_quality_index = previous_readings[i - 1].air_quality_index + (AIR_QUALITY_WORSENING_MARGIN - 0.05);
  }
}


/**
 * @brief Construct a list of prior readings that presents a hazard due to increasing air pressure.
 * 
 * @param previous_readings the previous readings list where the test data will be stored.
 */
void create_rising_pressure_hazard(ambient_info_t previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY]) {
  ambient_info_t initial_values = {-2.4, 13.7, 3.0, 7.5, 267.77};
  previous_readings[0] = initial_values;
  for (int i = 1; i < LENGTH_PREVIOUS_READINGS_ARRAY; i++) {
    previous_readings[i].temperature = previous_readings[i - 1].temperature + (TEMPERATURE_INCREASE_MARGIN - 0.05);
    previous_readings[i].humidity = previous_readings[i - 1].humidity + (HUMIDITY_INCREASE_MARGIN - 0.05);
    previous_readings[i].pressure = previous_readings[i - 1].pressure + (PRESSURE_INCREASE_MARGIN);
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
    previous_readings[i].pressure = previous_readings[i - 1].pressure + (PRESSURE_INCREASE_MARGIN - 0.05);
    previous_readings[i].air_quality_index = previous_readings[i - 1].air_quality_index + (AIR_QUALITY_WORSENING_MARGIN);
  }
}