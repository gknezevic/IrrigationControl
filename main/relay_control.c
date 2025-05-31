#include <stdio.h>
#include "relay_control.h"
#include "uart_config.h"
#include "esp_log.h"
#include "utils.h"

// Control a single relay
void control_relay(uint8_t relay_num, bool state) {
    uint8_t command[8];
    uint8_t response[BUF_SIZE];
    uint8_t resp_length;
    
    command[0] = MODBUS_ADDRESS;         
    command[1] = TURN_ON_RELAY;          
    command[2] = 0x00;                   
    command[3] = relay_num - 1;          
    command[4] = state ? 0xFF : 0x00;    
    command[5] = 0x00;
    
    uint16_t crc = calculate_crc(command, 6);
    command[6] = crc & 0xFF;
    command[7] = (crc >> 8) & 0xFF;
    
    ESP_LOGI(TAG, "Sending command to turn relay %d %s", relay_num, state ? "ON" : "OFF");
    
    if (send_modbus_command(command, 8, response, &resp_length) == ESP_OK) {
        ESP_LOGI(TAG, "Response received (%d bytes)", resp_length);
        for (int i = 0; i < resp_length; i++) {
            printf("0x%02X ", response[i]);
        }
        printf("\n");
    }
}

// Set a single relay
void set_relay_control_mode(uint8_t relay_num) {
    uint8_t command[8];
    uint8_t response[BUF_SIZE];
    uint8_t resp_length;
    
    command[0] = MODBUS_ADDRESS;         
    command[1] = 0x06;              
    command[2] = 0x10;                   
    command[3] = relay_num - 1;          
    command[4] = 0x00;                  
    command[5] = 0x00;
    
    uint16_t crc = calculate_crc(command, 6);
    command[6] = crc & 0xFF;
    command[7] = (crc >> 8) & 0xFF;
    
    if (send_modbus_command(command, 8, response, &resp_length) == ESP_OK) {
        ESP_LOGI(TAG, "Response received (%d bytes)", resp_length);
        for (int i = 0; i < resp_length; i++) {
            printf("0x%02X ", response[i]);
        }
        printf("\n");
    }
}

// Read relay status
void read_relay_status() {
    uint8_t command[8];
    uint8_t response[BUF_SIZE];
    uint8_t resp_length;
    
    command[0] = MODBUS_ADDRESS;  
    command[1] = READ_RELAY;      
    command[2] = 0x10;           
    command[3] = 0x00;           
    command[4] = 0x00;           
    command[5] = 0x08;           
    
    uint16_t crc = calculate_crc(command, 6);
    command[6] = crc & 0xFF;
    command[7] = (crc >> 8) & 0xFF;
    
    ESP_LOGI(TAG, "Sending command to read relay status");
    
    if (send_modbus_command(command, 8, response, &resp_length) == ESP_OK) {
        if (resp_length >= 5) {
            ESP_LOGI(TAG, "Relay status response received (%d bytes)", resp_length);
            for (int i = 0; i < resp_length; i++) {
                printf("0x%02X ", response[i]);
            }
        } else {
            ESP_LOGE(TAG, "Invalid response length");
        }
    }
}

void relay_control_task(void *arg) {
    configure_uart();
    
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

// Send Modbus command and wait for response
esp_err_t send_modbus_command(uint8_t *command, uint8_t cmd_length, uint8_t *response, uint8_t *resp_length) {
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