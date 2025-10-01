#ifndef WIRELESS_STATION_H
#define WIRELESS_STATION_H


// Aliases.
typedef uint pin_t;


// Structs.
typedef struct {
    float humidity;
    float temp_celsius;
} dht22_reading;


// Program constants.
#define DHT22_PIN 0
#define SDA_PIN 4
#define SCL_PIN 5
#define MAX_TIMINGS 85
#define I2C_BAUDRATE 100000
#define LIGHT_SENSOR_I2C_ADDRESS 0x23
#define BH1750_CONT_H_RES_MODE 0x10
#define BOARD_ADC_RESOLUTION 4096


// Sensor specifications constants.
#define MQ_7_LOWER_LIMIT 20
#define MQ_7_UPPER_LIMIT 2000


// Function declarations for the different sensors.
int read_temperature_and_humidity(dht22_reading *reading);
int read_light_intensity(float *reading);


#endif