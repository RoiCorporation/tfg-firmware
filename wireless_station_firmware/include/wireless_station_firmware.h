#ifndef WIRELESS_STATION_FIRMWARE_H
#define WIRELESS_STATION_FIRMWARE_H


// Aliases.
typedef unsigned int pin_t;


// Structs.
typedef struct {
    float temperature;
    float humidity;
    float pressure;
    float air_quality_index;
    float light_intensity;
} ambient_info_t;


// Program constants.
#define AMBIENT_INFO_FIELD_COUNT 5
#define DHT22_PIN 0
#define BUZZER_PIN 15
#define SDA_PIN 4
#define SCL_PIN 5
#define MAX_TIMINGS 85
#define I2C_BAUDRATE 100000
#define LIGHT_SENSOR_I2C_ADDRESS 0x23
#define BH1750_CONT_H_RES_MODE 0x10
#define BOARD_ADC_RESOLUTION 4096
#define MINUTE_IN_MILLISECONDS 60000
#define LENGTH_PREVIOUS_READINGS_ARRAY 5
#define TEMPERATURE_INCREASE_MARGIN 0.2
#define HUMIDITY_INCREASE_MARGIN 2.0
#define PRESSURE_INCREASE_MARGIN 2.0
#define AIR_QUALITY_WORSENING_MARGIN 2.0
#define EPSILON 1e-5


// Function declarations for the different sensors and other devices used.
void initialize_board();
void initialize_i2c_bus();
int read_temperature_and_humidity(ambient_info_t *reading);
int read_light_intensity(ambient_info_t *reading);
unsigned int analyze_hazards(ambient_info_t previous_readings[]);
void play_hazard_alert(unsigned int hazard_code);


#endif