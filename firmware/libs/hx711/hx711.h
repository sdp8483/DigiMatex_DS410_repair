#ifndef HX711_H_
#define HX711_H_

#include <stddef.h>
#include "main.h"

#define HX711_CH_A_GAIN_128 1   		// 1: channel A, gain factor 128
#define HX711_CH_B_GAIN_32  2   		// 2: channel B, gain factor 32
#define HX711_CH_A_GAIN_64  3   		// 3: channel A, gain factor 64

typedef struct __HX711_HandleTypeDef {
	GPIO_TypeDef *GPIO_port_Sck;	// PD_SCK Port
	GPIO_TypeDef *GPIO_port_Data;	// DOUT Port
	uint16_t GPIO_pin_Sck;			// PD_SCK Pin
	uint16_t GPIO_pin_Data;			// DOUT Pin
	int32_t offset;					// value from calling tare function
	uint8_t gain;					// sets gain and channel used
} HX711_HandleTypeDef;

typedef struct __HX711_ScalingTypeDef {
	uint32_t p1_value;				// raw HX711 value at point 1, X1
	uint32_t p1_eng;				// grams at point 1, Y1
	uint32_t p2_value;				// raw HX711 value at point 2, X2
	uint32_t p2_eng;				// grams at point 2, Y2
	float m;							// slope of transfer function
	float b;							// intercept of transfer function
}HX711_ScalingTypeDef;

#define DATA_READ HAL_GPIO_ReadPin(hhx711->GPIO_port_Data, hhx711->GPIO_pin_Data)
#define SCK_SET	 HAL_GPIO_WritePin(hhx711->GPIO_port_Sck, hhx711->GPIO_pin_Sck, GPIO_PIN_SET)
#define SCK_RESET HAL_GPIO_WritePin(hhx711->GPIO_port_Sck, hhx711->GPIO_pin_Sck, GPIO_PIN_RESET)

void HX711_Init(HX711_HandleTypeDef *hhx711, GPIO_TypeDef *SCK_port, uint16_t SCK_pin,
		GPIO_TypeDef *DATA_port, uint16_t DATA_pin, uint8_t gain);

int32_t HX711_Value(HX711_HandleTypeDef *hhx711);
int32_t HX711_Average(HX711_HandleTypeDef *hhx711, uint8_t times);
void HX711_Tare(HX711_HandleTypeDef *hhx711, uint8_t times);
void HX711_UpdateScaling(HX711_ScalingTypeDef *shx711);
int32_t HX711_Calculate(HX711_ScalingTypeDef *shx711, uint32_t value);

#endif /* HX711_H_ */
