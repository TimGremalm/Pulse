#include "controller.h"

#include <stdint.h>
#include <math.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "dmx.h"

#define Fixture_ChannelStart 10
#define Fixture_Channels 10
#define Fixtures 2

/*
Fixture Eurolite LED PARty RGBW Spot
#define Channel_Red 1
#define Channel_Green 2
#define Channel_Blue 3
#define Channel_White 4
#define Channel_Dimmer 5
#define Channel_Amber 7
#define Channel_UV 8
*/

/*
Fixture Cameo Flat Pro Flood 600 IP65 CLFLOOD600IP65
*/
#define Channel_Dimmer 1
#define Channel_Strobe 2
#define Channel_Duration 3
#define Channel_Red 4
#define Channel_Green 5
#define Channel_Blue 6
#define Channel_White 7
#define Channel_Amber 8
#define Channel_UV 9

#define min(x, y) (((x) < (y)) ? (x) : (y))
#define max(a,b) \
	({ __typeof__ (a) _a = (a); \
	__typeof__ (b) _b = (b); \
	_a > _b ? _a : _b; })

float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static const char *TAG = "Controller";

void setChannels(uint8_t channel, uint8_t value) {
	uint8_t ch = 0;
	for (uint8_t i = 0; i < Fixtures; i++) {
		ch = Fixture_ChannelStart + (i * Fixture_Channels) + (channel - 1);
		// ESP_LOGI(TAG, "Channel: %d Value: %d", ch, value);
		dmx_data[ch] = value;
	}
}

typedef struct {
	uint32_t seconds;
	float max;
} wave_t;

void controllertask(void *pvParameters) {
	ESP_LOGI(TAG, "Start");
	setChannels(Channel_Red, 255);

	float color_Red = 0.00;
	float color_Green = 0.32;
	float color_Blue = 0.32;
	float color_White = 0.72;
	float color_Amber = 0.32;
	float color_UV = 0.64;

	float level_dim_min = 0.30;
	float level_dim_max = 1.00;
	float level_color_min = 0.5;
	float level_color_max = 1.0;

	uint32_t time_common = 12000;
	wave_t waves[] = {
		{ .seconds = time_common, .max = 0.50 },
		{ .seconds = time_common, .max = 0.75 },
		{ .seconds = time_common, .max = 1.00 },
		{ .seconds = time_common, .max = 0.75 },
		{ .seconds = time_common, .max = 0.50 },
		{ .seconds = time_common, .max = 0.00 },
	};
	uint8_t number_of_waves = sizeof(waves)/sizeof(wave_t);
	uint8_t current_wave = 0;

	uint32_t loop_sleep_ms = 10;
	uint32_t wave_elapsed_ms = 0;

	while(1) {
		uint32_t wave_cycles = waves[current_wave].seconds / loop_sleep_ms;
		uint32_t wave_elapsed_increase = waves[current_wave].seconds / wave_cycles;
		uint32_t wave_elapsed_half = waves[current_wave].seconds / 2;
		wave_elapsed_ms += wave_elapsed_increase;
		// Reset for next wave
		if (wave_elapsed_ms > waves[current_wave].seconds) {
			wave_elapsed_ms = 0;
			current_wave += 1;
			// Reset to first wave
			if (current_wave >= number_of_waves) {
				current_wave = 0;
			}
			ESP_LOGI(TAG, "Wave: %d/%d Length: %dms Max level: %f", current_wave, number_of_waves,
															waves[current_wave].seconds, waves[current_wave].max);
		}

		// First half of wave, ramp up. Then ramp down.
		float cycle_level = 0;
		if (wave_elapsed_ms < wave_elapsed_half) {
			cycle_level = ((float)wave_elapsed_ms) / wave_elapsed_half;
		} else {
			cycle_level = 1.0 - (((float)(wave_elapsed_ms - wave_elapsed_half)) / wave_elapsed_half);
		}

		// Soften curve by using the seconfs half (pi to 2pi) of cosine.
		// Offset by 1 to avoid negative range, and divide by 2 to range from 0 to 1.
		float level_soft = (cosf(M_PI + (cycle_level * M_PI)) + 1) / 2;
		// ESP_LOGI(TAG, "Wave %d Elapsed %d Level %f Soft %f", current_wave, wave_elapsed_ms, cycle_level, level_soft);

		// Apply wave level.
		float level_wave = level_soft * waves[current_wave].max;

		// Apply min- and max-levels form dimmer- and color-channels.
		float level_dim = mapf(level_wave, 0.0, 1.0, level_dim_min, level_dim_max);
		float level_color = mapf(level_wave, 0.0, 1.0, level_color_min, level_color_max);
		// ESP_LOGI(TAG, "Color Level %f after %f", level_color, level_color * 255 * color_Red);

		// Convert float 0-1 values to 8-bit levels, also adjust for individual colors.
		setChannels(Channel_Dimmer, level_dim * 255);
		setChannels(Channel_Red, (uint8_t)(level_color * 255 * color_Red));
		setChannels(Channel_Green, (uint8_t)(level_color * 255 * color_Green));
		setChannels(Channel_Blue, (uint8_t)(level_color * 255 * color_Blue));
		setChannels(Channel_White, (uint8_t)(level_color * 255 * color_White));
		setChannels(Channel_Amber, (uint8_t)(level_color * 255 * color_Amber));
		setChannels(Channel_UV, (uint8_t)(level_color * 255 * color_UV));
		vTaskDelay(loop_sleep_ms / portTICK_PERIOD_MS);
	}
}

