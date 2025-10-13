#ifndef ERRORS_H
#define ERRORS_H


// Identifiers for errors with the sensors.
#define TEMP_HUMIDITY_READ_ERROR 0x20  // Couldn't read the temperature or humidity values.
#define LIGHT_SENSOR_READ_ERROR 0x21  // Couldn't obtain the value read by the light sensor through I2C.


// Identifiers for the different hazards.
#define TEMPERATURE_RISING_HAZARD 0x31
#define HUMIDITY_RISING_HAZARD 0x32
#define PRESSURE_RISING_HAZARD 0x33
#define AIR_QUALITY_WORSENING_HAZARD 0x34


#endif