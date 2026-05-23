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


#include "iaq.h"


void calculate_iaq(float humidity, float gas_concentration, float *iaq) {
    float humidity_score = 0, gas_score = 0;

    if (humidity >= 38 && humidity <= 42) // Humidity +/-5% around optimum.
        humidity_score = 0.25 * 100;
    else { // Humidity is sub-optimal.
        if (humidity < 38)
            humidity_score = 0.25 / HUMIDITY_REFERENCE * humidity * 100;
        else {
            humidity_score = ((-0.25 / (100 - HUMIDITY_REFERENCE) * humidity) + 0.416666) * 100;
        }
    }

    gas_score = (0.75 / (GAS_UPPER_LIMIT - GAS_LOWER_LIMIT) * GAS_REFERENCE - (GAS_LOWER_LIMIT * (0.75 / (GAS_UPPER_LIMIT - GAS_LOWER_LIMIT)))) * 100.00;
    if (gas_score > 75)
        gas_score = 75; // Sometimes gas readings can go outside of expected scale maximum.
    if (gas_score < 0)
        gas_score = 0; // Sometimes gas readings can go outside of expected scale minimum.

    *iaq = humidity_score + gas_score;
}
