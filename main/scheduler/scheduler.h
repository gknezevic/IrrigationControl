#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

void turn_off_with_delay_callback(TimerHandle_t xTimer);
void turn_off_with_delay();
void turn_off_relays_task(void *pvParameters);

#endif // SCHEDULER_H