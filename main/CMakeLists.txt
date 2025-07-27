idf_component_register(SRCS "main.c" "relay_control.c" "uart_config.c" "utils.c" "wifi/wifi_control.c" "mqtt/mqtt_control.c"
                    INCLUDE_DIRS "." "wifi" "mqtt")