#include <stdio.h>
#include "esp_log.h"
#include "scheduler.h"
#include "relay_control.h"
#include "freertos/timers.h"

static const char *TAG_SCHEDULER = "SCHEDULER";

void turn_off_with_delay_callback(TimerHandle_t xTimer) {
    ESP_LOGI(TAG_SCHEDULER, "Turning off relay after delay");
    xTimerDelete(xTimer, 0);

    xTaskCreate(turn_off_relays_task, "RelayCtrl", 2048, NULL, 2, NULL);
    
    ESP_LOGI(TAG_SCHEDULER, "Relay turned off");
}

void turn_off_relays_task(void *pvParameters) {
    ESP_LOGI(TAG_SCHEDULER, "Stopping pump and turning off relay all relays");
    control_relay(8, false);
    for (int i = 1; i <= 7; i++) {
        control_relay(i, false);
    }
    vTaskDelete(NULL);
}


void turn_off_with_delay() {
    TimerHandle_t timer = xTimerCreate("TurnOffTimer", pdMS_TO_TICKS(2*60*60*1000), pdFALSE, 0, turn_off_with_delay_callback);
    if (timer == NULL) {
        ESP_LOGE(TAG_SCHEDULER, "Failed to create timer");
        return;
    }
    if (xTimerStart(timer, 0) != pdPASS) {
        ESP_LOGE(TAG_SCHEDULER, "Failed to start timer");
        xTimerDelete(timer, 0);
        return;
    }
    ESP_LOGI(TAG_SCHEDULER, "Timer started to turn off relay after delay");
}