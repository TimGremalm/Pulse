#include "controller.h"

#include <stdint.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "dmx.h"

static const char *TAG = "Controller";

void controllertask(void *pvParameters) {
	ESP_LOGI(TAG, "Start");
	dmx_data[1] = 255;
	while(1) {
		// ESP_LOGI(TAG, "DMX Channel 1: %d", dmx_data[1]);
		dmx_data[5] = 255;
		vTaskDelay(200 / portTICK_PERIOD_MS);
	}
}

