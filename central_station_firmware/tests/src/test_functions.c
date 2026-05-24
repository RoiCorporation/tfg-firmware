#include "central_station_firmware.h"
#include "errors.h"


/**
 * @brief Reset the previous readings list by setting all the elements to zero.
 *
 * @param previous_readings the previous readings list to reset.
 */
void clean_previous_readings_list(ambient_info_t previous_readings[]) {
    for (int i = 0; i < LENGTH_PREVIOUS_READINGS_ARRAY; i++) {
        previous_readings[i].temperature = 0.0;
        previous_readings[i].humidity = 0.0;
        previous_readings[i].light_intensity = 0.0;
        previous_readings[i].air_pressure = 0.0;
        previous_readings[i].air_quality_index = 0.0;
        previous_readings[i].carbon_monoxide_concentration = 0.0;
        previous_readings[i].methane_concentration = 0.0;
        previous_readings[i].propane_concentration = 0.0;
        previous_readings[i].hydrogen_gas_concentration = 0.0;
    }
}


/**
 * @brief Construct a list of prior readings that presents a hazard due to increasing temperature.
 *
 * @param previous_readings the previous readings list where the test data will be stored.
 */
void create_rising_temperature_hazard(ambient_info_t previous_readings[]) {
    ambient_info_t initial_values = {
        .temperature = -2.4,
        .humidity = 13.7,
        .light_intensity = 3.0,
        .air_pressure = 7.5,
        .air_quality_index = 59.27,
        .carbon_monoxide_concentration = 15.0,
        .methane_concentration = 256.0,
        .propane_concentration = 182.62,
        .hydrogen_gas_concentration = 3557.82};
    previous_readings[0] = initial_values;

    for (int i = 1; i < LENGTH_PREVIOUS_READINGS_ARRAY; i++) {
        previous_readings[i].temperature =
            previous_readings[i - 1].temperature +
            (TEMPERATURE_INCREASE_MARGIN);
        previous_readings[i].humidity =
            previous_readings[i - 1].humidity +
            (HUMIDITY_INCREASE_MARGIN - 0.05);
        previous_readings[i].air_pressure =
            previous_readings[i - 1].air_pressure +
            (-56.6 - 0.05);
        previous_readings[i].air_quality_index =
            previous_readings[i - 1].air_quality_index +
            (AIR_QUALITY_WORSENING_MARGIN - 0.05);
        previous_readings[i].carbon_monoxide_concentration =
            previous_readings[i - 1].carbon_monoxide_concentration +
            (CARBON_MONOXIDE_WORSENING_MARGIN - 0.05);
        previous_readings[i].methane_concentration =
            previous_readings[i - 1].methane_concentration +
            (METHANE_WORSENING_MARGIN - 0.05);
        previous_readings[i].propane_concentration =
            previous_readings[i - 1].propane_concentration +
            (PROPANE_WORSENING_MARGIN - 0.05);
        previous_readings[i].hydrogen_gas_concentration =
            previous_readings[i - 1].hydrogen_gas_concentration +
            (HYDROGEN_GAS_WORSENING_MARGIN - 0.05);
    }
}


/**
 * @brief Construct a list of prior readings that presents a hazard due to increasing humidity.
 *
 * @param previous_readings the previous readings list where the test data will be stored.
 */
void create_rising_humidity_hazard(ambient_info_t previous_readings[]) {
    ambient_info_t initial_values = {
        .temperature = -2.4,
        .humidity = 13.7,
        .light_intensity = 3.0,
        .air_pressure = 7.5,
        .air_quality_index = 59.27,
        .carbon_monoxide_concentration = 15.0,
        .methane_concentration = 256.0,
        .propane_concentration = 182.62,
        .hydrogen_gas_concentration = 3557.82};
    previous_readings[0] = initial_values;

    for (int i = 1; i < LENGTH_PREVIOUS_READINGS_ARRAY; i++) {
        previous_readings[i].temperature =
            previous_readings[i - 1].temperature +
            (TEMPERATURE_INCREASE_MARGIN - 0.05);
        previous_readings[i].humidity =
            previous_readings[i - 1].humidity +
            (HUMIDITY_INCREASE_MARGIN);
        previous_readings[i].air_pressure =
            previous_readings[i - 1].air_pressure +
            (-52.6 - 0.05);
        previous_readings[i].air_quality_index =
            previous_readings[i - 1].air_quality_index +
            (AIR_QUALITY_WORSENING_MARGIN - 0.05);
        previous_readings[i].carbon_monoxide_concentration =
            previous_readings[i - 1].carbon_monoxide_concentration +
            (CARBON_MONOXIDE_WORSENING_MARGIN - 0.05);
        previous_readings[i].methane_concentration =
            previous_readings[i - 1].methane_concentration +
            (METHANE_WORSENING_MARGIN - 0.05);
        previous_readings[i].propane_concentration =
            previous_readings[i - 1].propane_concentration +
            (PROPANE_WORSENING_MARGIN - 0.05);
        previous_readings[i].hydrogen_gas_concentration =
            previous_readings[i - 1].hydrogen_gas_concentration +
            (HYDROGEN_GAS_WORSENING_MARGIN - 0.05);
    }
}


/**
 * @brief Construct a list of prior readings that presents a hazard due to air quality
 * continuously worsening.
 *
 * @param previous_readings the previous readings list where the test data will be stored.
 */
void create_worsening_air_quality_hazard(ambient_info_t previous_readings[]) {
    ambient_info_t initial_values = {
        .temperature = -2.4,
        .humidity = 13.7,
        .light_intensity = 3.0,
        .air_pressure = 7.5,
        .air_quality_index = 59.27,
        .carbon_monoxide_concentration = 15.0,
        .methane_concentration = 256.0,
        .propane_concentration = 182.62,
        .hydrogen_gas_concentration = 3557.82};
    previous_readings[0] = initial_values;

    for (int i = 1; i < LENGTH_PREVIOUS_READINGS_ARRAY; i++) {
        previous_readings[i].temperature =
            previous_readings[i - 1].temperature +
            (TEMPERATURE_INCREASE_MARGIN - 0.05);
        previous_readings[i].humidity =
            previous_readings[i - 1].humidity +
            (HUMIDITY_INCREASE_MARGIN - 0.05);
        previous_readings[i].air_pressure =
            previous_readings[i - 1].air_pressure +
            (-56.6 - 0.05);
        previous_readings[i].air_quality_index =
            previous_readings[i - 1].air_quality_index +
            (AIR_QUALITY_WORSENING_MARGIN);
        previous_readings[i].carbon_monoxide_concentration =
            previous_readings[i - 1].carbon_monoxide_concentration +
            (CARBON_MONOXIDE_WORSENING_MARGIN - 0.05);
        previous_readings[i].methane_concentration =
            previous_readings[i - 1].methane_concentration +
            (METHANE_WORSENING_MARGIN - 0.05);
        previous_readings[i].propane_concentration =
            previous_readings[i - 1].propane_concentration +
            (PROPANE_WORSENING_MARGIN - 0.05);
        previous_readings[i].hydrogen_gas_concentration =
            previous_readings[i - 1].hydrogen_gas_concentration +
            (HYDROGEN_GAS_WORSENING_MARGIN - 0.05);
    }
}


/**
 * @brief Construct a list of prior readings that presents a hazard due to the
 * carbon monoxide concentration continuously worsening.
 *
 * @param previous_readings the previous readings list where the test data will be stored.
 */
void create_worsening_carbon_monoxide_hazard(ambient_info_t previous_readings[]) {
    ambient_info_t initial_values = {
        .temperature = -2.4,
        .humidity = 13.7,
        .light_intensity = 3.0,
        .air_pressure = 7.5,
        .air_quality_index = 59.27,
        .carbon_monoxide_concentration = 15.0,
        .methane_concentration = 256.0,
        .propane_concentration = 182.62,
        .hydrogen_gas_concentration = 3557.82};
    previous_readings[0] = initial_values;

    for (int i = 1; i < LENGTH_PREVIOUS_READINGS_ARRAY; i++) {
        previous_readings[i].temperature =
            previous_readings[i - 1].temperature +
            (TEMPERATURE_INCREASE_MARGIN - 0.05);
        previous_readings[i].humidity =
            previous_readings[i - 1].humidity +
            (HUMIDITY_INCREASE_MARGIN - 0.05);
        previous_readings[i].light_intensity =
            previous_readings[i - 1].light_intensity +
            0.1;
        previous_readings[i].air_pressure =
            previous_readings[i - 1].air_pressure +
            (-56.6 - 0.05);
        previous_readings[i].air_quality_index =
            previous_readings[i - 1].air_quality_index +
            (AIR_QUALITY_WORSENING_MARGIN - 0.05);
        previous_readings[i].carbon_monoxide_concentration =
            previous_readings[i - 1].carbon_monoxide_concentration +
            (CARBON_MONOXIDE_WORSENING_MARGIN);
        previous_readings[i].methane_concentration =
            previous_readings[i - 1].methane_concentration +
            (METHANE_WORSENING_MARGIN - 0.05);
        previous_readings[i].propane_concentration =
            previous_readings[i - 1].propane_concentration +
            (PROPANE_WORSENING_MARGIN - 0.05);
        previous_readings[i].hydrogen_gas_concentration =
            previous_readings[i - 1].hydrogen_gas_concentration +
            (HYDROGEN_GAS_WORSENING_MARGIN - 0.05);
    }
}


/**
 * @brief Construct a list of prior readings that presents a hazard due to the
 * carbon monoxide concentration continuously worsening.
 *
 * @param previous_readings the previous readings list where the test data will be stored.
 */
void create_worsening_methane_hazard(ambient_info_t previous_readings[]) {
    ambient_info_t initial_values = {
        .temperature = -2.4,
        .humidity = 13.7,
        .light_intensity = 3.0,
        .air_pressure = 7.5,
        .air_quality_index = 59.27,
        .carbon_monoxide_concentration = 15.0,
        .methane_concentration = 256.0,
        .propane_concentration = 182.62,
        .hydrogen_gas_concentration = 3557.82};
    previous_readings[0] = initial_values;

    for (int i = 1; i < LENGTH_PREVIOUS_READINGS_ARRAY; i++) {
        previous_readings[i].temperature =
            previous_readings[i - 1].temperature +
            (TEMPERATURE_INCREASE_MARGIN - 0.05);
        previous_readings[i].humidity =
            previous_readings[i - 1].humidity +
            (HUMIDITY_INCREASE_MARGIN - 0.05);
        previous_readings[i].light_intensity =
            previous_readings[i - 1].light_intensity +
            0.1;
        previous_readings[i].air_pressure =
            previous_readings[i - 1].air_pressure +
            (-56.6 - 0.05);
        previous_readings[i].air_quality_index =
            previous_readings[i - 1].air_quality_index +
            (AIR_QUALITY_WORSENING_MARGIN - 0.05);
        previous_readings[i].carbon_monoxide_concentration =
            previous_readings[i - 1].carbon_monoxide_concentration +
            (CARBON_MONOXIDE_WORSENING_MARGIN - 0.05);
        previous_readings[i].methane_concentration =
            previous_readings[i - 1].methane_concentration +
            (METHANE_WORSENING_MARGIN);
        previous_readings[i].propane_concentration =
            previous_readings[i - 1].propane_concentration +
            (PROPANE_WORSENING_MARGIN - 0.05);
        previous_readings[i].hydrogen_gas_concentration =
            previous_readings[i - 1].hydrogen_gas_concentration +
            (HYDROGEN_GAS_WORSENING_MARGIN - 0.05);
    }
}


/**
 * @brief Construct a list of prior readings that presents a hazard due to the
 * propane concentration continuously worsening.
 *
 * @param previous_readings the previous readings list where the test data will be stored.
 */
void create_worsening_propane_hazard(ambient_info_t previous_readings[]) {
    ambient_info_t initial_values = {
        .temperature = -2.4,
        .humidity = 13.7,
        .light_intensity = 3.0,
        .air_pressure = 7.5,
        .air_quality_index = 59.27,
        .carbon_monoxide_concentration = 15.0,
        .methane_concentration = 256.0,
        .propane_concentration = 182.62,
        .hydrogen_gas_concentration = 3557.82};
    previous_readings[0] = initial_values;

    for (int i = 1; i < LENGTH_PREVIOUS_READINGS_ARRAY; i++) {
        previous_readings[i].temperature =
            previous_readings[i - 1].temperature +
            (TEMPERATURE_INCREASE_MARGIN - 0.05);
        previous_readings[i].humidity =
            previous_readings[i - 1].humidity +
            (HUMIDITY_INCREASE_MARGIN - 0.05);
        previous_readings[i].light_intensity =
            previous_readings[i - 1].light_intensity +
            0.1;
        previous_readings[i].air_pressure =
            previous_readings[i - 1].air_pressure +
            (-56.6 - 0.05);
        previous_readings[i].air_quality_index =
            previous_readings[i - 1].air_quality_index +
            (AIR_QUALITY_WORSENING_MARGIN - 0.05);
        previous_readings[i].carbon_monoxide_concentration =
            previous_readings[i - 1].carbon_monoxide_concentration +
            (CARBON_MONOXIDE_WORSENING_MARGIN - 0.05);
        previous_readings[i].methane_concentration =
            previous_readings[i - 1].methane_concentration +
            (METHANE_WORSENING_MARGIN - 0.05);
        previous_readings[i].propane_concentration =
            previous_readings[i - 1].propane_concentration +
            (PROPANE_WORSENING_MARGIN);
        previous_readings[i].hydrogen_gas_concentration =
            previous_readings[i - 1].hydrogen_gas_concentration +
            (HYDROGEN_GAS_WORSENING_MARGIN - 0.05);
    }
}


/**
 * @brief Construct a list of prior readings that presents a hazard due to the
 * hydrogen gas concentration continuously worsening.
 *
 * @param previous_readings the previous readings list where the test data will be stored.
 */
void create_worsening_hydrogen_gas_hazard(ambient_info_t previous_readings[]) {
    ambient_info_t initial_values = {
        .temperature = -2.4,
        .humidity = 13.7,
        .light_intensity = 3.0,
        .air_pressure = 7.5,
        .air_quality_index = 59.27,
        .carbon_monoxide_concentration = 15.0,
        .methane_concentration = 256.0,
        .propane_concentration = 182.62,
        .hydrogen_gas_concentration = 3557.82};
    previous_readings[0] = initial_values;

    for (int i = 1; i < LENGTH_PREVIOUS_READINGS_ARRAY; i++) {
        previous_readings[i].temperature =
            previous_readings[i - 1].temperature +
            (TEMPERATURE_INCREASE_MARGIN - 0.05);
        previous_readings[i].humidity =
            previous_readings[i - 1].humidity +
            (HUMIDITY_INCREASE_MARGIN - 0.05);
        previous_readings[i].light_intensity =
            previous_readings[i - 1].light_intensity +
            0.1;
        previous_readings[i].air_pressure =
            previous_readings[i - 1].air_pressure +
            (-56.6 - 0.05);
        previous_readings[i].air_quality_index =
            previous_readings[i - 1].air_quality_index +
            (AIR_QUALITY_WORSENING_MARGIN - 0.05);
        previous_readings[i].carbon_monoxide_concentration =
            previous_readings[i - 1].carbon_monoxide_concentration +
            (CARBON_MONOXIDE_WORSENING_MARGIN - 0.05);
        previous_readings[i].methane_concentration =
            previous_readings[i - 1].methane_concentration +
            (METHANE_WORSENING_MARGIN - 0.05);
        previous_readings[i].propane_concentration =
            previous_readings[i - 1].propane_concentration +
            (PROPANE_WORSENING_MARGIN - 0.05);
        previous_readings[i].hydrogen_gas_concentration =
            previous_readings[i - 1].hydrogen_gas_concentration +
            (HYDROGEN_GAS_WORSENING_MARGIN);
    }
}


/**
 * @brief Construct a list of prior readings that doesn't present any hazards.
 *
 * @param previous_readings the previous readings list where the test data will be stored.
 */
void create_correct_previous_readings_list(ambient_info_t previous_readings[]) {
    ambient_info_t initial_values = {
        .temperature = -2.4,
        .humidity = 13.7,
        .light_intensity = 3.0,
        .air_pressure = 7.5,
        .air_quality_index = 59.27,
        .carbon_monoxide_concentration = 15.0,
        .methane_concentration = 256.0,
        .propane_concentration = 182.62,
        .hydrogen_gas_concentration = 3557.82};
    previous_readings[0] = initial_values;

    for (int i = 1; i < LENGTH_PREVIOUS_READINGS_ARRAY; i++) {
        previous_readings[i].temperature =
            previous_readings[i - 1].temperature +
            (TEMPERATURE_INCREASE_MARGIN - 0.05);
        previous_readings[i].humidity =
            previous_readings[i - 1].humidity +
            (HUMIDITY_INCREASE_MARGIN - 0.05);
        previous_readings[i].light_intensity =
            previous_readings[i - 1].light_intensity +
            0.1;
        previous_readings[i].air_pressure =
            previous_readings[i - 1].air_pressure +
            (-56.6 - 0.05);
        previous_readings[i].air_quality_index =
            previous_readings[i - 1].air_quality_index +
            (AIR_QUALITY_WORSENING_MARGIN - 0.05);
        previous_readings[i].carbon_monoxide_concentration =
            previous_readings[i - 1].carbon_monoxide_concentration +
            (CARBON_MONOXIDE_WORSENING_MARGIN - 0.05);
        previous_readings[i].methane_concentration =
            previous_readings[i - 1].methane_concentration +
            (METHANE_WORSENING_MARGIN - 0.05);
        previous_readings[i].propane_concentration =
            previous_readings[i - 1].propane_concentration +
            (PROPANE_WORSENING_MARGIN - 0.05);
        previous_readings[i].hydrogen_gas_concentration =
            previous_readings[i - 1].hydrogen_gas_concentration +
            (HYDROGEN_GAS_WORSENING_MARGIN - 0.05);
    }
}
