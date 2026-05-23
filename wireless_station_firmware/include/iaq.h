/**
 * This code has been adapted from the BME680-Example library by
 * David Bird.
 *
 * Full credit for the original implementation belongs to the
 * original author.
 *
 * Original repository:
 * https://github.com/G6EJD/BME680-Example
 *
 * Modifications were made solely to integrate the code into
 * this project.
 */


#ifndef IAQ_H
#define IAQ_H


#include "wireless_station_firmware.h"


#define GAS_LOWER_LIMIT 10000  // Bad air quality limit.
#define GAS_UPPER_LIMIT 300000 // Good air quality limit.
#define HUMIDITY_WEIGHTING 0.25
#define GAS_WEIGHTING 0.75
#define GAS_REFERENCE 2500
#define HUMIDITY_REFERENCE 40


void calculate_iaq(float humidity, float gas_concentration, float *iaq);


#endif
