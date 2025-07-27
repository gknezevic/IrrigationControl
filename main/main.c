#include <stdio.h>
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "relay_control.h"
#include "uart_config.h"
#include "wifi/wifi_control.h"

void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    //xTaskCreate(relay_control_task, "relay_control_task", 4096, NULL, 10, NULL);
    xTaskCreate(wifi_task, "wifi_task", 4096, NULL, 10, NULL);
}