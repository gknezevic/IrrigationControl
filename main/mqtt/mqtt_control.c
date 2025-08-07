#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "relay_control.h"

#define MQTT_BROKER_URI "mqtt://broker.hivemq.com"

static const char *TAG_MQTT = "MQTT";
static const char *TOPIC = "/esp32/relay";
esp_mqtt_client_handle_t client = NULL;

void mqtt_event_handler(void *handler_args, esp_event_base_t base,
    int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG_MQTT, "MQTT connected");
            esp_mqtt_client_publish(client, TOPIC, "ESP32 connected", 0, 1, 0);
            esp_mqtt_client_subscribe(client, TOPIC, 0);
            ESP_LOGI(TAG_MQTT, "Subscribed to topic: %s", TOPIC);
            break;
        case MQTT_EVENT_DATA: {
            ESP_LOGI(TAG_MQTT, "MQTT message received:");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);

            // Copy and parse message
            char msg[32] = {0};
            strncpy(msg, event->data, event->data_len);

            char cmd[8];
            int relay_num = 0;
            if (sscanf(msg, "%7s %d", cmd, &relay_num) == 2 && relay_num >= 0 && relay_num <= 7) {
                ESP_LOGI(TAG_MQTT, "Message: %s", msg);
                ESP_LOGI(TAG_MQTT, "relay_num %d", relay_num);
                if (strcmp(cmd, "STOP") == 0) {
                    ESP_LOGI(TAG_MQTT, "Stopping pump and turning off relay all relays");
                    control_relay(8, false);
                    // Wait for confirmation before turning off all other relays
                    for (int i = 1; i <= 7; i++) {
                        control_relay(i, false);
                    }
                    esp_mqtt_client_publish(client, TOPIC, read_relay_status(), 0, 1, 0);
                } else if (strcmp(cmd, "START") == 0) {
                    ESP_LOGI(TAG_MQTT, "Starting pump and turning on relay %d", relay_num);
                    control_relay(relay_num, true);
                    // Wait for confirmation before turning on pump
                    control_relay(8, true);
                    esp_mqtt_client_publish(client, TOPIC, read_relay_status(), 0, 1, 0);
                } else if (strcmp(cmd, "STATUS") == 0) {
                    ESP_LOGI(TAG_MQTT, "GET Status of relays");
                    esp_mqtt_client_publish(client, TOPIC, read_relay_status(), 0, 1, 0);
                } else {
                    ESP_LOGW(TAG_MQTT, "Unknown command: %s", cmd);
                }
            } else {
                ESP_LOGW(TAG_MQTT, "Invalid message format: %s", msg);
            }

            break;
        }
        default:
            break;
    }
}

void mqtt_app_start(void) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = MQTT_BROKER_URI,
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}
