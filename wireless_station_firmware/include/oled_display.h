#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H


#include "wireless_station_firmware.h"


#define MAX_DISPLAY_DIGITS_BUFFER_SIZE 15


void display_temperature(ssd1306_t *oled_display, float temperature);
void display_humidity(ssd1306_t *oled_display, float humidity);
void display_light_intensity(ssd1306_t *oled_display, float light_intensity);
void display_air_pressure(ssd1306_t *oled_display, float air_pressure);
void display_air_quality_index(ssd1306_t *oled_display, float air_quality_index);


#endif
