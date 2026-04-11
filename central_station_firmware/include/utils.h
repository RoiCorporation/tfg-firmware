#ifndef UTILS_H
#define UTILS_H


#include "central_station_firmware.h"


int8_t hex_value(char c);
void initialize_station_id_to_address_buffer(
    station_id_address_map_t station_id_to_nrf24_address_buffer[],
    size_t buffer_size,
    size_t address_size
);
int8_t hex_string_to_bytes(const char *hex, uint8_t *out, size_t out_size);
int8_t bytes_to_string_uuid(
    uint8_t id_in_bytes[],
    char *uuid_string[],
    size_t size_bytes,
    size_t uuid_string_size
);


#endif
