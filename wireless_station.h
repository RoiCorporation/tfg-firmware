#ifndef WIRELESS_STATION_H
#define WIRELESS_STATION_H


// Aliases.
typedef uint pin_t;


// Structs.
typedef struct {
    float humidity;
    float temp_celsius;
} dht22_reading;


// Constants.
const pin_t DHT22_PIN = 0;
const pin_t MQ_7_PIN = 26;
const pin_t SDA_PIN = 4;
const pin_t SCL_PIN = 5;
const pin_t MAX_TIMINGS = 85;
const pin_t I2C_BAUDRATE = 100000;
const pin_t LIGHT_SENSOR_I2C_ADDRESS = 0x23;
const pin_t BH1750_CONT_H_RES_MODE = 0x10;


// Function declarations for the different sensors.
void read_temperature_and_humidity(dht22_reading *result);


#endif