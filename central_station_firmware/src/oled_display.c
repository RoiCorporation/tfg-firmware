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
    ssd1306_draw_string(oled_display, 100, 50, 2, "C");
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
    if (humidity > 10) {
        icon_x_offset = 6;
        value_x_offset = 46;
    }
    if (humidity == 100) {
        icon_x_offset = 2;
        value_x_offset = 34;
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
    ssd1306_draw_string(oled_display, 100, 50, 2, "RH");
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
    snprintf(light_intensity_string, sizeof(light_intensity_string), "%.0f",
        light_intensity);

    // Update the icon and value offsets according to the magnitude of
    // the light intensity.
    if ((light_intensity / 10000) > 1) {
        icon_x_offset = 0;
        value_x_offset = 48;
        value_y_offset = 24;
        value_scale = 2;
    }
    else if ((light_intensity / 1000) > 1) {
        icon_x_offset = 6;
        value_x_offset = 40;
    }
    else if ((light_intensity / 100) > 1) {
        icon_x_offset = 12;
        value_x_offset = 54;
    }
    else if ((light_intensity / 10) > 1) {
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
    ssd1306_draw_string(oled_display, 100, 50, 2, "lx");
    ssd1306_show(oled_display);
}


/**
 * @brief Displays the given air pressure in a 128x64 OLED display.
 * 
 * @param oled_display pointer to the OLED display driver.
 * @param air_pressure value of the air pressure to display.
 */
void display_air_pressure(ssd1306_t *oled_display, float air_pressure) {

}


/**
 * @brief Displays the given Air Quality Index (AQI) in a 128x64 OLED display.
 * 
 * @param oled_display pointer to the OLED display driver.
 * @param air_quality_index value of the air_quality_index to display.
 */
void display_air_quality_index(ssd1306_t *oled_display, float air_quality_index) {

}


/**
 * @brief Displays the given carbon monoxide (CO) concentration in a 128x64
 * OLED display.
 * 
 * @param oled_display pointer to the OLED display driver.
 * @param carbon_monoxide_concentration value of the carbon monoxide concentration
 * to display.
 */
void display_carbon_monoxide_concentration(
    ssd1306_t *oled_display,
    float carbon_monoxide_concentration
) {

}


/**
 * @brief Displays the given methane (CH4) concentration in a 128x64 OLED display.
 * 
 * @param oled_display pointer to the OLED display driver.
 * @param methane_concentration value of the methane concentration to display.
 */
void display_methane_concentration(
    ssd1306_t *oled_display,
    float methane_concentration
) {

}


/**
 * @brief Displays the given propane (C3H8) concentration in a 128x64 OLED display.
 * 
 * @param oled_display pointer to the OLED display driver.
 * @param propane_concentration value of the propane concentration to display.
 */
void display_propane_concentration(
    ssd1306_t *oled_display,
    float propane_concentration
) {

}


/**
 * @brief Displays the given alcohol concentration in a 128x64 OLED display.
 * 
 * @param oled_display pointer to the OLED display driver.
 * @param alcohol_concentration value of the alcohol concentration to display.
 */
void display_alcohol_concentration(
    ssd1306_t *oled_display,
    float alcohol_concentration
) {

}


/**
 * @brief Displays the given hydrogen gas (H2) concentration in a 128x64 OLED
 * display.
 * 
 * @param oled_display pointer to the OLED display driver.
 * @param hydrogen_gas_concentration value of the hydrogen gas concentration to
 * display.
 */
void display_hydrogen_gas_concentration(
    ssd1306_t *oled_display,
    float hydrogen_gas_concentration
) {

}
