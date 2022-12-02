#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "dmx.h"
#include "controller.h"

// Init global variable
uint8_t dmx_data[513];
uint16_t hall_data;

static const char *TAG = "Main";

void app_main(void) {
	ESP_LOGI(TAG, "Start Pulse");

	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	xTaskCreatePinnedToCore(&controllertask, "controller_task", 4096, NULL, 5, NULL, 0);
	xTaskCreatePinnedToCore(&dmxtask, "dmx_task", 4096, NULL, 5, NULL, 1);
}

