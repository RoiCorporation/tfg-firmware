#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "monocypher.h"


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
 * @brief Convert a uint32_t value into an array containing its 4 bytes.
 * 
 * @param value uint32_t number to convert to an array.
 * @param array uint8_t array that will store the bytes that make up the value
 * passed. Its size must be 4 and the bytes are arranged in big-endian.
 */
void uint32_to_u8_be(uint32_t value, uint8_t array[]) {
    array[0] = (uint8_t)((value >> 24) & 0xff);
    array[1] = (uint8_t)((value >> 16) & 0xff);
    array[2] = (uint8_t)((value >> 8)  & 0xff);
    array[3] = (uint8_t)( value        & 0xff);
}


/**
 * @brief Key-Derivation Function used to create a key for the AES encryption
 * module from the shared secret generated after the ECDH key exchange.
 * 
 * @param output_key uint8_t array that will contain the generated key.
 * @param output_key_size size_t size of the key in bytes.
 * @param shared_secret uint8_t array that contains the shared secret generated
 * after the ECDH key exchange.
 * @param shared_secret_size size_t size of the shared secret in bytes.
 * @param salt uint8_t array containing the salt used to increase the randomness
 * of the generated key.
 * @param salt_size size_t size of the salt in bytes.
 */
void kdf(
    uint8_t *output_key,
    size_t output_key_size,
    uint8_t *shared_secret,
    size_t shared_secret_size,
    uint8_t *salt,
    size_t salt_size
){
    // Nonce must be 8 bytes.
    uint8_t chacha_nonce[8] = {'I', '5', 'S', 'K', 'L', 'Y', '0', '8'};

	// Extract.
	uint8_t prk[32];
	crypto_blake2b_keyed(
        prk,
        sizeof(prk),
        salt,
        salt_size,
        shared_secret,
        shared_secret_size
    );

	// Expand.
	crypto_chacha20_djb(
        output_key,
        NULL,
        output_key_size,
        prk,
        chacha_nonce,
        0
    );
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
