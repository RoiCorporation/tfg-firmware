#include <stdlib.h>
#include <string.h>
#include "pico/rand.h"
#include "utils.h"


/**
 * @brief Convert a hex character to its integer value.
 * 
 * @param c hex character ('0'-'9', 'a'-'f', 'A'-'F').
 * @return int8_t value 0–15, or -1 if invalid.
 */
int8_t hex_value(char c) {
    if (c >= '0' && c <= '9')
        return (int8_t)(c - '0');

    if (c >= 'a' && c <= 'f')
        return (int8_t)(c - 'a' + 10);

    if (c >= 'A' && c <= 'F')
        return (int8_t)(c - 'A' + 10);

    return (int8_t)-1;
}


/**
 * @brief Initialize the buffer that maps each associated station ID with
 * the particular address of each NRF24L01 module data pipe. Those addresses
 * are generated randomly in this method, ensuring no two of them match.
 * 
 * @param station_id_to_nrf24_address_buffer structure that stores the mapping
 * between the ID of each of the associated wireless stations and their respective
 * receiving address in this central station's NRF24 module.
 * @param buffer_size amount of data pipes of the module, should be 6.
 * @param address_size length (in bytes) of the addresses, should be 5.
 */
void initialize_station_id_to_address_buffer(
    station_id_address_map_t station_id_to_nrf24_address_buffer[],
    size_t buffer_size,
    size_t address_size
) {
    
    // Position inside an address array of the Least Significant Byte (LSB).
    uint8_t lsb_pos = address_size - 1;

    // Initialize all the station ID slots available to NULL.
    for (int i = 0; i < buffer_size; i++)
        station_id_to_nrf24_address_buffer[i].associated_station_id = NULL;
    
    // Array of size 256 that marks which bytes have been selected randomly for
    // the LSB of each address. It's used after generating all addresses to ensure
    // that no two of them are equal and that all of their LSBs are different.
    uint8_t lsbs_generated[256] = { 0x00 };

    // Initialize all addresses.
    for (int i = 0; i < buffer_size; i++) {

        // Generate the full addresses for the first two data pipes (data pipes 0 and 1).
        if (i < 2) {
            for (int j = 0; j < address_size; j++)
                station_id_to_nrf24_address_buffer[i].nrf24l01_address[j] = (uint8_t)get_rand_32();
        }

        // For the remaining data pipes, only generate the LSB.
        else {
            station_id_to_nrf24_address_buffer[i].nrf24l01_address[lsb_pos] = (uint8_t)get_rand_32();
        }

        // If the LSB of the current address isn't equal to another address' LSB,
        // mark it as assigned in the marks array. In any other case, repeat this
        // iteration to generate a new full address or LSB, depending on which
        // data pipe the address is for.
        uint8_t lsb_value_before = lsbs_generated[station_id_to_nrf24_address_buffer[i].nrf24l01_address[lsb_pos]];
        if (lsb_value_before != 0)
            i--;
        else
            lsbs_generated[station_id_to_nrf24_address_buffer[i].nrf24l01_address[lsb_pos]] = 1;
    }
}


/**
 * @brief Convert hex string to bytes (2 hex chars → 1 byte).
 * 
 * Supports odd-length strings by treating the first nibble as low 
 * (e.g. "f" → 0x0F). Also skips hyphens.
 * 
 * @param hex input hex string (null-terminated).
 * @param out output buffer.
 * @param out_size size of output buffer.
 * @return int8_t number of bytes written, or -1 on error.
 */
int8_t hex_string_to_bytes(const char *hex, uint8_t *out, size_t out_size) {
    if (!hex || !out)
        return (int8_t)-1;

    size_t j = 0;
    int high_nibble = -1;

    for (size_t i = 0; hex[i] != '\0'; i++) {
        char c = hex[i];

        // Skip dashes.
        if (c == '-')
            continue;

        int val = hex_value(c);
        if (val < 0)
            return (int8_t)-1;

        if (high_nibble < 0) {
            high_nibble = val;
        } 
        else {
            if (j >= out_size)
                return (int8_t)-1;

            out[j++] = (uint8_t)((high_nibble << 4) | val);
            high_nibble = -1;
        }
    }

    // Handle odd number of hex digits (last nibble).
    if (high_nibble >= 0) {
        if (j >= out_size)
            return (int8_t)-1;
        out[j++] = (uint8_t)high_nibble; // → 0x0X
    }

    return (int8_t)j;
}


/**
 * @brief Convert raw UUID bytes to canonical UUID string.
 *
 * @param id_in_bytes input byte array.
 * @param uuid_string pointer to the output buffer.
 * @param size_bytes number of input bytes, must be 16.
 * @param uuid_string_size size of output buffer, must be 37.
 * @return int8_t 0 on success, -1 on error.
 */
int8_t bytes_to_string_uuid(
    uint8_t id_in_bytes[],
    char *uuid_string[],
    size_t size_bytes,
    size_t uuid_string_size
) {
    static const char hex[] = "0123456789abcdef";
    size_t j = 0;

    if (*uuid_string == NULL)
        *uuid_string = malloc(uuid_string_size * sizeof(char));

    if (!id_in_bytes || !uuid_string)
        return (int8_t)-1;
    if (size_bytes != 16)
        return (int8_t)-1;
    if (uuid_string_size < 37)
        return (int8_t)-1;

    for (size_t i = 0; i < size_bytes; i++) {
        // Insert dashes before bytes 4, 6, 8, 10
        if (i == 4 || i == 6 || i == 8 || i == 10) {
            (*uuid_string)[j++] = '-';
        }

        (*uuid_string)[j++] = hex[(id_in_bytes[i] >> 4) & 0x0F];
        (*uuid_string)[j++] = hex[id_in_bytes[i] & 0x0F];
    }

    (*uuid_string)[j] = '\0';
    return (int8_t)0;
}
