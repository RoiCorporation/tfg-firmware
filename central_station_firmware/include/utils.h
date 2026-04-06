#ifndef UTILS_H
#define UTILS_H


int8_t hex_value(char c);
int8_t hex_string_to_bytes(const char *hex, uint8_t *out, size_t out_size);
int8_t bytes_to_string_uuid(
    uint8_t id_in_bytes[],
    char uuid_string[],
    size_t size_bytes,
    size_t uuid_string_size
);


#endif
