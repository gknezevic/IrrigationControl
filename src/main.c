#include "driver/uart.h"
#include "esp_log.h"

#define MODBUS_UART UART_NUM_2
#define MODBUS_TX_PIN 17
#define MODBUS_RX_PIN 16
#define MODBUS_BAUD_RATE 9600

static const char *TAG = "ModbusRTU";

void modbus_rtu_init() {
    uart_config_t uart_config = {
        .baud_rate = MODBUS_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_param_config(MODBUS_UART, &uart_config);
    uart_set_pin(MODBUS_UART, MODBUS_TX_PIN, MODBUS_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(MODBUS_UART, 1024, 0, 0, NULL, 0);
}

void modbus_rtu_send(uint8_t *data, size_t len) {
    uart_write_bytes(MODBUS_UART, (const char *)data, len);
}

void app_main() {
    modbus_rtu_init();
    uint8_t modbus_request[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x0A}; // Example Modbus RTU request
    modbus_rtu_send(modbus_request, sizeof(modbus_request));
    ESP_LOGI(TAG, "Modbus request sent");
}