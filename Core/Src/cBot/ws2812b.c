/*
 * ws2812b.c
 *
 * Copyright (C) 2021  Stefan Hoermann (mail@stefan-hoermann.de)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <ws2812b.h>

extern DMA_HandleTypeDef hdma_tim2_ch1;

ws2812b_t *currentTransfer = NULL;

void ws2812b_init(ws2812b_t *ws2812b, uint16_t size, TIM_HandleTypeDef *htim, uint32_t channel) {
	// setup
	ws2812b->size = size;
	ws2812b->rgbData = malloc(size * 24 + 1);
	ws2812b->timingLow = (uint8_t)((htim->Init.Period + 1) / 3);
	ws2812b->timingHigh = (uint8_t)((htim->Init.Period + 1) * 2 / 3);
	ws2812b->htim = htim;
	ws2812b->channel = channel;

	// clear rgb data
	ws2812b_clear(ws2812b);

	// append reset timing to the end of PWM sequence
	ws2812b->rgbData[size * 24] = 0;


	//HAL_PWM_RegisterCallback(PWM_PulseFinishedCallback
	//HAL_DMA_RegisterCallback(&hdma_tim2_ch1, HAL_DMA_XFER_CPLT_CB_ID, ws2812b_dma_completed);
    //0x8001348
	//hdma_tim2_ch1.XferCpltCallback = ws2812b_dma_completed;
}

void ws2812b_deInit(ws2812b_t *ws2812b) {
	free(ws2812b->rgbData);
}

void ws2812b_clear(ws2812b_t *ws2812b) {
	for ( int i = ws2812b->size * 24 -1; i >= 0; i-- ) {
		ws2812b->rgbData[i] = ws2812b->timingLow;
	}
}

uint32_t ws2812b_colorRGB(uint8_t r, uint8_t g, uint8_t b) {
	return CONSTEXPR_COLOR(r, g, b);
}

uint32_t ws2812b_colorHSV(uint16_t h, uint8_t s, uint8_t v) {
	uint32_t r = 0, g = 0, b = 0, base;

	// value (0..1535)
	if ( h < 256 ) {
		r = 255;
		g = h;
		b = 0;
	} else if ( h < 512 ) {
		r = 255 - (h - 256);
		g = 255;
		b = 0;
	} else if ( h < 768 ) {
		r = 0;
		g = 255;
		b = h - 512;
	} else if ( h < 1024 ) {
		r = 0;
		g = 255 - (h - 768);
		b = 255;
	} else if ( h < 1280 ) {
		r = h - 1024;
		g = 0;
		b = 255;
	} else if ( h < 1536 ) {
		r = 255;
		g = 0;
		b = 255 - (h - 1280);
	}

	// saturation (0..100%)
	base = (100 - s) * 255 / 100;
	r = r * s / 100 + base;
	g = g * s / 100 + base;
	b = b * s / 100 + base;

	// value (0..100%)
	r = (r * v) / 100;
	g = (g * v) / 100;
	b = (b * v) / 100;

	return CONSTEXPR_COLOR(r,g,b); //(uint32_t)g << 16 | (uint32_t)r << 8 | (uint32_t)b;
}

void ws2812b_setColor(ws2812b_t *ws2812b, uint16_t id, uint32_t color) {
	uint8_t *buf = ws2812b->rgbData + (id % ws2812b->size) * 24;
	for ( uint32_t i  = 0; i < 24; i++) {
		*(buf++) = (color & 0x800000) ? ws2812b->timingHigh : ws2812b->timingLow;
		color = color << 1;
	}
}

void ws2812b_update(ws2812b_t *ws2812b) {
	HAL_TIM_Base_Stop(ws2812b->htim);
	__HAL_TIM_SET_COUNTER(ws2812b->htim, 0);
	HAL_TIM_PWM_Start_DMA(ws2812b->htim, ws2812b->channel, (uint32_t *)ws2812b->rgbData, ws2812b->size * 24 + 1);
	currentTransfer = ws2812b;
	HAL_TIM_Base_Start(ws2812b->htim);
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
	currentTransfer = NULL;
};
