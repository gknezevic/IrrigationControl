#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

// Function to calculate CRC for Modbus RTU
uint16_t calculate_crc(uint8_t *data, uint8_t length);
char *binary_chars(uint8_t input);

#endif // UTILS_H