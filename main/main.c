#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "driver/uart.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "driver/gpio.h"

#define TAG "WAVESHARE_MODBUS"

// UART Configuration (update these in your Kconfig)
#define UART_TXD_PIN    (CONFIG_ECHO_UART_TXD)
#define UART_RXD_PIN    (CONFIG_ECHO_UART_RXD)
#define UART_RTS_PIN    (CONFIG_ECHO_UART_RTS)  // For RS485 DE/RE control
#define UART_BAUD_RATE  (9600)                  // Modbus standard baud rate
#define UART_PORT_NUM   (CONFIG_ECHO_UART_PORT_NUM)
#define BUF_SIZE        (256)

// Modbus RTU Commands for Waveshare Relay
#define MODBUS_ADDRESS  (0x01)  // Default address is 1 (change if needed)
#define TURN_ON_RELAY   (0x05)  // Modbus function 05 - Write Single Coil
#define TURN_OFF_RELAY  (0x05)  // Same function, different data
#define READ_RELAY      (0x03)  // Modbus function 01 - Read Coils

void configure_max485() {
    gpio_set_direction(UART_RTS_PIN, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_level(UART_RTS_PIN, 0);  // Start in receive mode (RE = 1, DE = 0)
}

void enable_uart_transmit() {
    gpio_set_level(UART_RTS_PIN, 1);  // DE = 1 (Transmit mode)
}

void disable_uart_transmit() {
    gpio_set_level(UART_RTS_PIN, 0);  // DE = 0 (Receive mode)
}

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

// Send Modbus command and wait for response
static esp_err_t send_modbus_command(uint8_t *command, uint8_t cmd_length, uint8_t *response, uint8_t *resp_length) {
    enable_uart_transmit();
    vTaskDelay(pdMS_TO_TICKS(100));  // Allow time for DE/RE to switch
    // Send the command
    uart_write_bytes(UART_PORT_NUM, (const char*)command, cmd_length);
    
    printf("TX Frame: ");
    for (int i = 0; i < 8; ++i) {
        printf("%02X ", command[i]);
    }
    printf("\n");
    //disable_uart_transmit();
    vTaskDelay(pdMS_TO_TICKS(100));  // Allow time for DE/RE to switch back
    // Wait for transmission to complete
    uart_wait_tx_done(UART_PORT_NUM, pdMS_TO_TICKS(100));
    
    // Read response with timeout
    int len = uart_read_bytes(UART_PORT_NUM, response, BUF_SIZE, pdMS_TO_TICKS(500));
    if (len <= 0) {
        ESP_LOGE(TAG, "No response from device");
        return ESP_FAIL;
    }
    
    *resp_length = len;
    return ESP_OK;
}

// Control a single relay
static void control_relay(uint8_t relay_num, bool state) {
    uint8_t command[8];
    uint8_t response[BUF_SIZE];
    uint8_t resp_length;
    
    // Build Modbus command
    command[0] = MODBUS_ADDRESS;         // Slave address
    command[1] = TURN_ON_RELAY;          // Function code
    command[2] = 0x00;                   // Relay number high byte (0x0000-0x0007 for 8 relays)
    command[3] = relay_num - 1;          // Relay number low byte (0-based)
    command[4] = state ? 0xFF : 0x00;    // ON: 0xFF00, OFF: 0x0000
    command[5] = 0x00;
    
    // Calculate CRC
    uint16_t crc = calculate_crc(command, 6);
    command[6] = crc & 0xFF;
    command[7] = (crc >> 8) & 0xFF;
    
    ESP_LOGI(TAG, "Sending command to turn relay %d %s", relay_num, state ? "ON" : "OFF");
    
    // Send command and check response
    if (send_modbus_command(command, 8, response, &resp_length) == ESP_OK) {
        ESP_LOGI(TAG, "Response received (%d bytes)", resp_length);
        for (int i = 0; i < resp_length; i++) {
            printf("0x%02X ", response[i]);
        }
        printf("\n");
    }
}

// Set a single relay
static void set_relay_control_mode(uint8_t relay_num) {
    uint8_t command[8];
    uint8_t response[BUF_SIZE];
    uint8_t resp_length;
    
    // Build Modbus command
    command[0] = MODBUS_ADDRESS;         // Slave address
    command[1] = 0x06;              // Function code
    command[2] = 0x10;                   // Relay number high byte (0x1000-0x1007 for 8 relays)
    command[3] = relay_num - 1;          // Relay number low byte (0-based)
    command[4] = 0x00;                  // Control mode
    command[5] = 0x00;
    
    // Calculate CRC
    uint16_t crc = calculate_crc(command, 6);
    command[6] = crc & 0xFF;
    command[7] = (crc >> 8) & 0xFF;
    
    // Send command and check response
    if (send_modbus_command(command, 8, response, &resp_length) == ESP_OK) {
        ESP_LOGI(TAG, "Response received (%d bytes)", resp_length);
        for (int i = 0; i < resp_length; i++) {
            printf("0x%02X ", response[i]);
        }
        printf("\n");
    }
}

// Read relay status
static void read_relay_status() {
    uint8_t command[8];
    uint8_t response[BUF_SIZE];
    uint8_t resp_length;
    
    // Build Modbus command
    command[0] = MODBUS_ADDRESS;  // Slave address
    command[1] = READ_RELAY;      // Function code
    command[2] = 0x10;           // Starting address high byte
    command[3] = 0x00;           // Starting address low byte (0x0000)
    command[4] = 0x00;           // Quantity of coils high byte
    command[5] = 0x08;           // Quantity of coils low byte (8 relays)
    
    // Calculate CRC
    uint16_t crc = calculate_crc(command, 6);
    command[6] = crc & 0xFF;
    command[7] = (crc >> 8) & 0xFF;
    
    ESP_LOGI(TAG, "Sending command to read relay status");
    
    // Send command and check response
    if (send_modbus_command(command, 8, response, &resp_length) == ESP_OK) {
        if (resp_length >= 5) {
            ESP_LOGI(TAG, "Relay status response received (%d bytes)", resp_length);
            for (int i = 0; i < resp_length; i++) {
                printf("0x%02X ", response[i]);
            }

            // uint8_t status = response[3];  // Status byte
            // ESP_LOGI(TAG, "Relay status: 0x%02X", status);
            
            // // Print individual relay status
            // for (int i = 0; i < 8; i++) {
            //     ESP_LOGI(TAG, "Relay %d: %s", i+1, (status & (1 << i)) ? "ON" : "OFF");
            // }
        } else {
            ESP_LOGE(TAG, "Invalid response length");
        }
    }
}

static void relay_control_task(void *arg) {
    // Configure UART
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,  // Modbus typically uses even parity
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    configure_max485();
    // Install UART driver
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, UART_TXD_PIN, UART_RXD_PIN, UART_RTS_PIN, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_set_mode(UART_PORT_NUM, UART_MODE_RS485_HALF_DUPLEX));

    
    
    ESP_LOGI(TAG, "UART initialized for Modbus RTU communication");
    
    // Main control loop
    while (1) {
        // Example usage: Cycle through relays
        for (int relay = 1; relay <= 8; relay++) {
            set_relay_control_mode(relay);
            vTaskDelay(pdMS_TO_TICKS(1000));
            // Turn relay on
            control_relay(relay, true);
            vTaskDelay(pdMS_TO_TICKS(1000));
            
            // Read status
            read_relay_status();
            vTaskDelay(pdMS_TO_TICKS(1000));
            
            // Turn relay off
            control_relay(relay, false);
            vTaskDelay(pdMS_TO_TICKS(1000));
            
            // Read status again
            read_relay_status();
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    }
}

void app_main(void) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Start relay control task
    xTaskCreate(relay_control_task, "relay_control_task", 4096, NULL, 10, NULL);
}

