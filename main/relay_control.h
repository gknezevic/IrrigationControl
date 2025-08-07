#ifndef RELAY_CONTROL_H
#define RELAY_CONTROL_H

#include <stdbool.h>
#include <stdint.h>
#include "glob.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"

#define MODBUS_ADDRESS 0x01
#define TURN_ON_RELAY 0x05
#define READ_RELAY 0x01
#define BUF_SIZE 256

// Function prototypes for relay control
void control_relay(uint8_t relay_num, bool state);
void set_relay_control_mode(uint8_t relay_num);
char *read_relay_status();
void relay_control_task(void *pvParameters);
esp_err_t send_modbus_command(uint8_t *command, uint8_t cmd_length, uint8_t *response, uint8_t *resp_length);

#endif // RELAY_CONTROL_H