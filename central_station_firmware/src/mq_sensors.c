/**
 * This code has been adapted from the following sources.
 *
 * - MQ7-Library, by swatish17
 * Original repository:
 * https://github.com/swatish17/MQ7-Library
 *
 * - lib-sensor-analog-mq4, by paveldhq
 * Original repository:
 * https://github.com/paveldhq/lib-sensor-analog-mq4
 *
 * Full credit for the original implementations belongs to the
 * original authors.
 *
 * Modifications were made solely to integrate the code into
 * this project.
 */

#include <math.h>
#include "mq_sensors.h"
#include "hardware/adc.h"


float calculate_adc_average(uint8_t adc_channel) {
    uint sum = 0;
    float avg = 0;

    adc_select_input(adc_channel);
    for (int i = 0; i < N_AVG_MEASUREMENTS; i++)
        sum += adc_read();

    return (float)sum / N_AVG_MEASUREMENTS;
}


float read_mq_7_co_ppm() {
    int adc_voltage = calculate_adc_average((uint8_t)0);
    float v_out = (float)adc_voltage * (3.3f / (ADC_RESOLUTION - 1));
    if (v_out < 0.01f)
        return 0;

    float rs = MQ_7_R_LOAD * ((3.3f - v_out) / v_out);
    float ratio = rs / MQ_7_R_0;
    return (float)(MQ_7_COEFFICIENT_A * pow(ratio, MQ_7_COEFFICIENT_B));
}


float read_mq_4_ch4_ppm() {
    int adc_voltage = calculate_adc_average(1);
    float v_out = ((float)adc_voltage * 3.3f) / 4095.0f;
    if (v_out < 0.01f)
        return 0;
    const float rl = 10.0f; // kΩ
    float rs_gas = rl * ((5.0f - v_out) / v_out);
    float ratio = rs_gas / MQ_4_R0;
    float ppm_log = (log10f(ratio) - MQ_4_COEFFICIENT_B) / MQ_4_COEFFICIENT_M;
    return (float)powf(10.0f, ppm_log);
}


float read_mq_6_c3h8_ppm() {
    int adc_voltage = calculate_adc_average(2);
    float v_out = ((float)adc_voltage * 3.3f) / 4095.0f;
    if (v_out < 0.01f)
        return 0;
    const float rl = 10.0f; // kΩ
    float rs_gas = rl * ((5.0f - v_out) / v_out);
    float ratio = rs_gas / MQ_6_R0;
    float ppm_log = (log10f(ratio) - MQ_6_COEFFICIENT_B) / MQ_6_COEFFICIENT_M;
    return (float)powf(10.0f, ppm_log);
}
