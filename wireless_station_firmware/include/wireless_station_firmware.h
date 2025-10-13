#ifndef WIRELESS_STATION_FIRMWARE_H
#define WIRELESS_STATION_FIRMWARE_H


// Aliases.
typedef uint pin_t;


// Structs.
typedef struct {
    float temperature;
    float humidity;
    float pressure;
    float air_quality_index;
    float light_intensity;
} ambient_info_t;


// Program constants.
#define DHT22_PIN 0
#define SDA_PIN 4
#define SCL_PIN 5
#define MAX_TIMINGS 85
#define I2C_BAUDRATE 100000
#define LIGHT_SENSOR_I2C_ADDRESS 0x23
#define BH1750_CONT_H_RES_MODE 0x10
#define BOARD_ADC_RESOLUTION 4096
#define MINUTE_IN_MILLISECONDS 60000
#define LENGTH_PREVIOUS_READINGS_ARRAY 5
#define TEMPERATURE_INCREASE_BUFFER 0.2
#define HUMIDITY_INCREASE_BUFFER 2.0
#define PRESSURE_INCREASE_BUFFER 2.0
#define AIR_QUALITY_WORSENING_BUFFER 2.0


// Function declarations for the different sensors.
void initialize_board();
int read_temperature_and_humidity(ambient_info_t *reading);
int read_light_intensity(ambient_info_t *reading);
int analyze_hazards(ambient_info_t previous_readings[]);


#endif