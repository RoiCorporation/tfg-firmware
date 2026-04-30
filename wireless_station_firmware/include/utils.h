#ifndef UTILS_H
#define UTILS_H


#include "wireless_station_firmware.h"


int8_t hex_value(char c);
int8_t hex_string_to_bytes(const char *hex, uint8_t *out, size_t out_size);
void uint32_to_u8_be(uint32_t value, uint8_t array[]);
void kdf(
    uint8_t *output_key,
    size_t output_key_size,
    uint8_t *shared_secret,
    size_t shared_secret_size,
    uint8_t *salt,
    size_t salt_size
);
int8_t bytes_to_string_uuid(
    uint8_t id_in_bytes[],
    char *uuid_string[],
    size_t size_bytes,
    size_t uuid_string_size
);


#endif
