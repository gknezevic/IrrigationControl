#include <stdio.h>
#include "driver/gpio.h"
#include "uart_config.h"
#include "esp_log.h"
#include "driver/uart.h"

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

void configure_uart() {
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
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, UART_BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, UART_TXD_PIN, UART_RXD_PIN, UART_RTS_PIN, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_set_mode(UART_PORT_NUM, UART_MODE_RS485_HALF_DUPLEX));

    
    
    ESP_LOGI(TAG, "UART initialized for Modbus RTU communication");
}