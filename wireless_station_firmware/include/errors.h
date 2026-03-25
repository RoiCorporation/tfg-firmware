#ifndef ERRORS_H
#define ERRORS_H


// Identifiers for errors with the sensors.
#define BME680_READ_ERROR 0x20  // Couldn't obtain some of the values read by the BME680 sensor.
#define LIGHT_SENSOR_READ_ERROR 0x21  // Couldn't obtain the value read by the light sensor through I2C.
#define DATA_TRANSMIT_ERROR 0x22  // Couldn't successfully transmit the ambient info through the radio module.
#define DATA_RECEIVE_ERROR 0x23  // Couldn't successfully receive the ambient info through the radio module.


#endif
