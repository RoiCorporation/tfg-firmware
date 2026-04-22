#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H


#include "central_station_firmware.h"


#define MAX_DISPLAY_DIGITS_BUFFER_SIZE 15


void display_temperature(ssd1306_t *oled_display, float temperature);
void display_humidity(ssd1306_t *oled_display, float humidity);
void display_light_intensity(ssd1306_t *oled_display, float light_intensity);
void display_air_pressure(ssd1306_t *oled_display, float air_pressure);
void display_air_quality_index(ssd1306_t *oled_display, float air_quality_index);
void display_carbon_monoxide_concentration(
    ssd1306_t *oled_display,
    float carbon_monoxide_concentration
);
void display_methane_concentration(
    ssd1306_t *oled_display,
    float methane_concentration
);
void display_propane_concentration(
    ssd1306_t *oled_display,
    float propane_concentration
);
void display_alcohol_concentration(
    ssd1306_t *oled_display,
    float alcohol_concentration
);
void display_hydrogen_gas_concentration(
    ssd1306_t *oled_display,
    float hydrogen_gas_concentration
);


#endif
