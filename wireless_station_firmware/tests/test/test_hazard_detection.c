#include "unity.h"
#include "wireless_station_firmware.h"
#include "hazards.h"
#include "test_functions.h"


TEST_SOURCE_FILE("hazards.c")


void setUp(void) {}
void tearDown(void) {}


void test_analyze_hazards_continuously_incrementing_parameters(void) {
    ambient_info_t previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY];

    // Test with a list of previous readings that poses a hazard due to
    // temperature rising continuously above the established safeguard.
    create_rising_temperature_hazard(previous_readings);
    TEST_ASSERT_EQUAL_INT(TEMPERATURE_RISING_HAZARD, analyze_hazards(previous_readings));

    // Test with a list of previous readings that poses a hazard due to
    // humidity rising continuously above the established safeguard.
    create_rising_humidity_hazard(previous_readings);
    TEST_ASSERT_EQUAL_INT(HUMIDITY_RISING_HAZARD, analyze_hazards(previous_readings));

    // Test with a list of previous readings that poses a hazard due to
    // air quality worsening continuously beyond the established safeguard.
    create_worsening_air_quality_hazard(previous_readings);
    TEST_ASSERT_EQUAL_INT(AIR_QUALITY_WORSENING_HAZARD, analyze_hazards(previous_readings));

    // Test with a list of previous readings that doesn't present any hazards whatsoever.
    create_correct_previous_readings_list(previous_readings);
    TEST_ASSERT_EQUAL_INT(0, analyze_hazards(previous_readings));
}


void test_analyze_hazards_breaking_incrementing_parameter_series(void) {
    ambient_info_t previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY];

    for (int i = 0; i < AMBIENT_INFO_FIELD_COUNT; i++) {
        switch (i) {
            case 0:
                create_rising_temperature_hazard(previous_readings);
                for (int j = 0; j < LENGTH_PREVIOUS_READINGS_ARRAY - 1; j++) {
                    previous_readings[j].temperature += 0.1;
                    TEST_ASSERT_EQUAL_INT(0, analyze_hazards(previous_readings));
                    previous_readings[j].temperature -= 0.1;
                }
                break;

            case 1:
                create_rising_humidity_hazard(previous_readings);
                for (int j = 0; j < LENGTH_PREVIOUS_READINGS_ARRAY - 1; j++) {
                    previous_readings[j].humidity += 0.1;
                    TEST_ASSERT_EQUAL_INT(0, analyze_hazards(previous_readings));
                    previous_readings[j].humidity -= 0.1;
                }
                break;

            case 2:
                // Unused, it doesn't make sense to establish a threshold for the light intensity.
                break;

            case 3:
                // Unused, it doesn't make sense to establish a threshold for the air pressure.
                break;

            case 4:
                create_worsening_air_quality_hazard(previous_readings);
                for (int j = 0; j < LENGTH_PREVIOUS_READINGS_ARRAY - 1; j++) {
                    previous_readings[j].air_quality_index += 0.1;
                    TEST_ASSERT_EQUAL_INT(0, analyze_hazards(previous_readings));
                    previous_readings[j].air_quality_index -= 0.1;
                }
                break;

            default:
                break;
        }

        clean_previous_readings_list(previous_readings); // Reset the previous readings list.
    }
}


void test_analyze_hazards_over_threshold_parameters(void) {
    ambient_info_t previous_readings[LENGTH_PREVIOUS_READINGS_ARRAY];
    create_correct_previous_readings_list(previous_readings);

    for (int i = 0; i < AMBIENT_INFO_FIELD_COUNT; i++) {
        switch (i) {
        case 0:
            for (int j = 0; j < LENGTH_PREVIOUS_READINGS_ARRAY - 1; j++) {
                float temperature_before = previous_readings[j].temperature;
                previous_readings[j].temperature = TEMPERATURE_HAZARD_THRESHOLD + 0.1;
                TEST_ASSERT_EQUAL_INT(TEMPERATURE_RISING_HAZARD, analyze_hazards(previous_readings));
                previous_readings[j].temperature = TEMPERATURE_HAZARD_THRESHOLD;
                TEST_ASSERT_EQUAL_INT(0, analyze_hazards(previous_readings));
                previous_readings[j].temperature = temperature_before;
            }
            break;

        case 1:
            // Unused, the humidity is relative and it doesn't make sense to establish
            // a threshold.
            break;

        case 2:
            // Unused, it doesn't make sense to establish a threshold for the light intensity.
            break;

        case 3:
            // Unused, it doesn't make sense to establish a threshold for the air pressure.
            break;

        case 4:
            for (int j = 0; j < LENGTH_PREVIOUS_READINGS_ARRAY - 1; j++) {
                float air_quality_index_before =
                    previous_readings[j].air_quality_index;
                previous_readings[j].air_quality_index =
                    AIR_QUALITY_HAZARD_THRESHOLD + 0.1;
                TEST_ASSERT_EQUAL_INT(
                    AIR_QUALITY_WORSENING_HAZARD,
                    analyze_hazards(previous_readings));
                previous_readings[j].air_quality_index =
                    AIR_QUALITY_HAZARD_THRESHOLD;
                TEST_ASSERT_EQUAL_INT(0, analyze_hazards(previous_readings));
                previous_readings[j].air_quality_index =
                    air_quality_index_before;
            }
            break;

        default:
            break;

        }

        clean_previous_readings_list(previous_readings); // Reset the previous readings list.
    }
}
