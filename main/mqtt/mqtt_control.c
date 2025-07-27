#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "mqtt_client.h"

#define WIFI_SSID "your-ssid"
#define WIFI_PASS "your-password"
#define MQTT_BROKER_URI "mqtt://broker.hivemq.com"

static const char *TAG = "MQTT_EXAMPLE";

// MQTT event handler
esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event) {
    esp_mqtt_client_handle_t client = event->client;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT connected");
            esp_mqtt_client_publish(client, "/esp32/relay", "Hello from ESP32", 0, 1, 0);
            break;
        default:
            break;
    }
    return ESP_OK;
}

void mqtt_app_start(void) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = MQTT_BROKER_URI,
        .event_handle = mqtt_event_handler_cb,
    };
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
}

// Wi-Fi init
void wifi_init(void) {
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS
        },
    };
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();
    esp_wifi_connect();
}

// void app_main(void) {
//     esp_err_t ret = nvs_flash_init();
//     if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
//         nvs_flash_erase();
//         ret = nvs_flash_init();
//     }

//     wifi_init();
//     vTaskDelay(pdMS_TO_TICKS(3000));  // Wait for Wi-Fi to connect
//     mqtt_app_start();
// }
