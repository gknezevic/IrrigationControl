// simple_connect.c
#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "wifi_control.h"

#include "esp_wifi.h"

#define TAG "WIFI_CONTROL"

#define WIFI_SSID "x"
#define WIFI_PASS "x"

void wifi_event_handler(void* arg, esp_event_base_t event_base, 
	int32_t event_id, void* event_data) {
	if (event_base == WIFI_EVENT) {
		if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
			esp_wifi_connect(); // Auto-reconnect
			ESP_LOGI(TAG, "Retrying connection...");
		}
	}
	else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
		ESP_LOGI(TAG, "Connected with IP: " IPSTR, IP2STR(&event->ip_info.ip));
	}
}

void wifi_task(void *pvParameters)
{
	esp_log_level_set("wifi", ESP_LOG_VERBOSE);
	esp_log_level_set("esp_netif", ESP_LOG_DEBUG);
	ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
	if (!sta_netif) {
		ESP_LOGE(TAG, "Failed to create STA interface");
		vTaskDelete(NULL);
	}
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
			.threshold.authmode = WIFI_AUTH_WPA2_PSK, 
			.pmf_cfg = {
				.capable = true, 
				.required = false
			}
        },
    };
	
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    ESP_ERROR_CHECK(esp_wifi_start());
	ESP_ERROR_CHECK(esp_wifi_connect());
}
