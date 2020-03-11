#include "hx711.h"

void HX711_Init(HX711_HandleTypeDef *hhx711, GPIO_TypeDef *SCK_port, uint16_t SCK_pin,
		GPIO_TypeDef *DATA_port, uint16_t DATA_pin, uint8_t gain) {
	/*
	 GPIO_InitTypeDef GPIO_InitStruct;
	 GPIO_InitStruct.Pin = hhx711->GPIO_pin_Sck;
	 GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	 GPIO_InitStruct.Pull = GPIO_NOPULL;
	 GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	 HAL_GPIO_Init(hhx711->GPIO_port_Sck, &GPIO_InitStruct);

	 GPIO_InitStruct.Pin = hhx711->GPIO_pin_Data;
	 GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	 GPIO_InitStruct.Pull = GPIO_PULLUP;
	 GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	 HAL_GPIO_Init(hhx711->GPIO_port_Data, &GPIO_InitStruct);
	 */

	hhx711->GPIO_port_Sck = SCK_port;
	hhx711->GPIO_pin_Sck = SCK_pin;
	hhx711->GPIO_port_Data = DATA_port;
	hhx711->GPIO_pin_Data = DATA_pin;
	hhx711->gain = gain;

	SCK_SET;
	HAL_Delay(50);
	SCK_RESET;
} // end HX711_Init

int32_t HX711_Value(HX711_HandleTypeDef *hhx711) {
	int32_t buffer;
	buffer = 0;

	while (DATA_READ == 1)
		;

	for (uint8_t i = 0; i < 24; i++) {
		SCK_SET;
		buffer = buffer << 1;
		if (DATA_READ) {
			buffer++;
		}
		SCK_RESET;
	}

	for (uint8_t i = 0; i < hhx711->gain; i++) {
		SCK_SET;
		SCK_RESET;
	}

	buffer = buffer ^ 0x800000;

	return buffer;
}

int32_t HX711_Average(HX711_HandleTypeDef *hhx711, uint8_t times) {
	int32_t sum = 0;
	for (uint8_t i = 0; i < times; i++) {
		sum += HX711_Value(hhx711);
	}

	return sum / times;
}

void HX711_Tare(HX711_HandleTypeDef *hhx711, uint8_t times) {
	hhx711->offset = HX711_Average(hhx711, times);
}

void HX711_UpdateScaling(HX711_ScalingTypeDef *shx711) {
	float m, b;

	// Calculate slope
	m = ((float) (shx711->p2_eng - shx711->p1_eng)) / ((float) (shx711->p2_value - shx711->p1_value));
	shx711->m = m;

	// calculate intercept
	b = shx711->p1_eng - (m*shx711->p1_value);
	shx711->b = b;
}

int32_t HX711_Calculate(HX711_ScalingTypeDef *shx711, uint32_t value) {

	return (shx711->m * value + shx711->b) + 0.5;
}
