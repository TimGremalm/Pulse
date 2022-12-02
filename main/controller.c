#include "controller.h"

#include <stdint.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "dmx.h"

#define Fixture_ChannelStart 10
#define Fixture_Channels 10
#define Fixtures 2

/*
Eurolite LED PARty RGBW Spot
*/
#define Channel_Red 1
#define Channel_Green 2
#define Channel_Blue 3
#define Channel_White 4
#define Channel_Dimmer 5

/*
Cameo Flat Pro Flood 600 IP65 CLFLOOD600IP65
#define Channel_Dimmer 1
#define Channel_Strobe 2
#define Channel_Duration 3
#define Channel_Red 4
#define Channel_Green 5
#define Channel_Blue 6
#define Channel_White 7
#define Channel_Amber 8
#define Channel_UV 9
*/

static const char *TAG = "Controller";

void setChannels(uint8_t channel, uint8_t value) {
	uint8_t ch = 0;
	for (uint8_t i = 0; i < Fixtures; i++) {
		ch = Fixture_ChannelStart + (i * Fixture_Channels) + (channel - 1);
		// ESP_LOGI(TAG, "Channel: %d Value: %d", ch, value);
		dmx_data[ch] = value;
	}
}

void controllertask(void *pvParameters) {
	ESP_LOGI(TAG, "Start");
	setChannels(Channel_Red, 255);
	while(1) {
		// ESP_LOGI(TAG, "DMX Channel 1: %d", dmx_data[1]);
		setChannels(Channel_Dimmer, 255);
		vTaskDelay(200 / portTICK_PERIOD_MS);
	}
}

