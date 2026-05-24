#ifndef MQ_SENSORS_H
#define MQ_SENSORS_H


#include "central_station_firmware.h"


/**
 * @brief Quick reminder:
 * 
 * MQ-7: Carbon monoxide (CO)
 * MQ-4: Methane (CH4)
 * MQ-6: Propane (C3H8)
 * 
 */


/* GENERAL */
#define ADC_RESOLUTION 4096
#define VDD 3.3   // MQ sensors require 5 volts for VDD.
#define N_AVG_MEASUREMENTS 10

/* MQ_7 */
#define MQ_7_COEFFICIENT_A 19.32
#define MQ_7_COEFFICIENT_B -0.64
#define MQ_7_R_LOAD 10.0   // Load resistance 10 Kilohms on the sensor potentiometer.
#define MQ_7_R_0 10.0

/* MQ_4 */
#define MQ_4_R0 11.820
#define MQ_4_COEFFICIENT_B 1.133
#define MQ_4_COEFFICIENT_M -0.318

/* MQ_6 */
#define MQ_6_R0 10.0f
#define MQ_6_COEFFICIENT_B 2.95f
#define MQ_6_COEFFICIENT_M -2.0f


/* FUNCTION DECLARATIONS */
float calculate_adc_average(uint8_t adc_channel);
float read_mq_7_co_ppm();
float read_mq_4_ch4_ppm();
float read_mq_6_c3h8_ppm();

#endif
