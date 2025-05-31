#ifndef UART_CONFIG_H
#define UART_CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include "glob.h"

// UART Configuration
#define UART_TXD_PIN    (CONFIG_ECHO_UART_TXD)
#define UART_RXD_PIN    (CONFIG_ECHO_UART_RXD)
#define UART_RTS_PIN    (CONFIG_ECHO_UART_RTS)  // For RS485 DE/RE control
#define UART_BAUD_RATE  (9600)                  // Modbus standard baud rate
#define UART_PORT_NUM   (CONFIG_ECHO_UART_PORT_NUM)
#define UART_BUF_SIZE 256

void configure_max485(void);
void enable_uart_transmit(void);
void disable_uart_transmit(void);
void configure_uart();

#endif // UART_CONFIG_H