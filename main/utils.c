#include <stdio.h>

// CRC calculation function
uint16_t calculate_crc(uint8_t *data, uint8_t length) {
    uint16_t crc = 0xFFFF;
    for (uint8_t pos = 0; pos < length; pos++) {
        crc ^= (uint16_t)data[pos];
        for (uint8_t i = 8; i != 0; i--) {
            if ((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

char *binary_chars(uint8_t input) {
    static char buffer[9]; // 8 bits + null terminator
    for (int i = 0; i < 8; i++) {
        buffer[i] = (input & (1 << (7 - i))) ? '1' : '0';
    }
    buffer[8] = '\0'; // Null-terminate the string
    return buffer;
}