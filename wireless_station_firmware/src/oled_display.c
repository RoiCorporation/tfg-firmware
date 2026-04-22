#include <stdio.h>
#include "oled_display.h"
#include "icons.h"


/**
 * @brief Displays the given temperature in a 128x64 OLED display.
 * 
 * @param oled_display pointer to the OLED display driver.
 * @param temperature value of the temperature to display.
 */
void display_temperature(ssd1306_t *oled_display, float temperature) {
    uint32_t icon_x_offset = 2, value_x_offset = 34;
    char temperature_string[MAX_DISPLAY_DIGITS_BUFFER_SIZE];
    snprintf(temperature_string, sizeof(temperature_string), "%.1f", temperature);

    // Update the icon and value offsets according to the magnitude of
    // the temperature.
    if (temperature > -10 && temperature < 0)
        value_x_offset = 40;
    else if (temperature > 0) {
        icon_x_offset = 12;
        if (temperature < 10)
            value_x_offset = 52;
        else
            value_x_offset = 46;
    }

    // Print the icon and temperature in the display with their respective offsets.
    ssd1306_clear(oled_display);
    ssd1306_bmp_show_image_with_offset(
        oled_display,
        media_bmp_temperature_icon_bmp_data,
        media_bmp_temperature_icon_bmp_size,
        icon_x_offset,
        14
    );
    ssd1306_draw_string(oled_display, value_x_offset, 20, 3, temperature_string);
    ssd1306_draw_string(oled_display, 100, 48, 2, "C");
    ssd1306_show(oled_display);
}


/**
 * @brief Displays the given humidity in a 128x64 OLED display.
 * 
 * @param oled_display pointer to the OLED display driver.
 * @param humidity value of the humidity to display.
 */
void display_humidity(ssd1306_t *oled_display, float humidity) {
    uint32_t icon_x_offset = 12, value_x_offset = 52;
    char humidity_string[MAX_DISPLAY_DIGITS_BUFFER_SIZE];
    snprintf(humidity_string, sizeof(humidity_string), "%.1f", humidity);

    // Update the icon and value offsets according to the magnitude of
    // the humidity.
    if (humidity == 100) {
        icon_x_offset = 2;
        value_x_offset = 34;
    }
    else if (humidity > 10) {
        icon_x_offset = 6;
        value_x_offset = 46;
    }

    // Print the icon and humidity in the display with their respective offsets.
    ssd1306_clear(oled_display);
    ssd1306_bmp_show_image_with_offset(
        oled_display,
        media_bmp_humidity_icon_bmp_data,
        media_bmp_humidity_icon_bmp_size,
        icon_x_offset,
        14
    );
    ssd1306_draw_string(oled_display, value_x_offset, 20, 3, humidity_string);
    ssd1306_draw_string(oled_display, 100, 48, 2, "RH");
    ssd1306_show(oled_display);
}


/**
 * @brief Displays the given light intensity in a 128x64 OLED display.
 * 
 * @param oled_display pointer to the OLED display driver.
 * @param light_intensity value of the light intensity to display.
 */
void display_light_intensity(ssd1306_t *oled_display, float light_intensity) {
    uint32_t icon_x_offset = 24, value_x_offset = 72, value_y_offset = 20,
    value_scale = 3;
    char light_intensity_string[MAX_DISPLAY_DIGITS_BUFFER_SIZE];
    snprintf(light_intensity_string, sizeof(light_intensity_string), "%d",
        (int)light_intensity);

    // Update the icon and value offsets according to the magnitude of
    // the light intensity.
    if (light_intensity >= 10000) {
        icon_x_offset = 6;
        value_x_offset = 48;
        value_y_offset = 24;
        value_scale = 2;
    }
    else if (light_intensity >= 1000) {
        icon_x_offset = 6;
        value_x_offset = 48;
    }
    else if (light_intensity >= 100) {
        icon_x_offset = 12;
        value_x_offset = 54;
    }
    else if (light_intensity >= 10) {
        icon_x_offset = 18;
        value_x_offset = 62;
    }

    // Print the icon and light intensity in the display with their respective
    // offsets.
    ssd1306_clear(oled_display);
    ssd1306_bmp_show_image_with_offset(
        oled_display,
        media_bmp_light_intensity_icon_bmp_data,
        media_bmp_light_intensity_icon_bmp_size,
        icon_x_offset,
        15
    );
    ssd1306_draw_string(
        oled_display,
        value_x_offset,
        value_y_offset,
        value_scale,
        light_intensity_string
    );
    ssd1306_draw_string(oled_display, 100, 48, 2, "lx");
    ssd1306_show(oled_display);
}


/**
 * @brief Displays the given air pressure in a 128x64 OLED display.
 * 
 * @param oled_display pointer to the OLED display driver.
 * @param air_pressure value of the air pressure to display.
 */
void display_air_pressure(ssd1306_t *oled_display, float air_pressure) {
    uint32_t value_x_offset = 39;
    char air_pressure_string[MAX_DISPLAY_DIGITS_BUFFER_SIZE];
    snprintf(air_pressure_string, sizeof(air_pressure_string), "%d",
        (int)air_pressure);

    // Update the value offset according to the magnitude of the air pressure.
    if (air_pressure >= 100000)
        value_x_offset = 34;

    // Print the title and air pressure in the display with their respective
    // offsets.
    ssd1306_clear(oled_display);
    ssd1306_draw_string(oled_display, 34, 18, 1, "Air pressure");
    ssd1306_draw_string(oled_display, value_x_offset, 30, 2, air_pressure_string);
    ssd1306_draw_string(oled_display, 100, 48, 2, "Pa");
    ssd1306_show(oled_display);
}


/**
 * @brief Displays the given Air Quality Index (AQI) in a 128x64 OLED display.
 * 
 * @param oled_display pointer to the OLED display driver.
 * @param air_quality_index value of the air_quality_index to display.
 */
void display_air_quality_index(ssd1306_t *oled_display, float air_quality_index) {
    uint32_t value_x_offset = 59;
    char air_quality_index_string[MAX_DISPLAY_DIGITS_BUFFER_SIZE];
    snprintf(air_quality_index_string, sizeof(air_quality_index_string), "%d",
        (int)air_quality_index);

    // Update the value offset according to the magnitude of the AQI.
    if (air_quality_index >= 100)
        value_x_offset = 49;
    else if (air_quality_index >= 10)
        value_x_offset = 54;

    // Print the title and air quality index in the display with their respective
    // offsets.
    ssd1306_clear(oled_display);
    ssd1306_draw_string(oled_display, 16, 18, 1, "Air Quality Index");
    ssd1306_draw_string(oled_display, value_x_offset, 30, 2,
        air_quality_index_string);
    ssd1306_draw_string(oled_display, 0, 48, 1, "Best: 0");
    ssd1306_draw_string(oled_display, 0, 56, 1, "Worst: 500");
    ssd1306_draw_string(oled_display, 90, 48, 2, "AQI");
    ssd1306_show(oled_display);
}
