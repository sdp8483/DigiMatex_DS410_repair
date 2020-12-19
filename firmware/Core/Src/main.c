/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "stdbool.h"
#include "usbd_cdc_if.h"

#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "hx711.h"
#include "eeprom_24LC02B.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define STR_BUFFER_SIZE 10
#define BUFFER_SIZE	15

#define EEPROM_P1_VALUE_ADDR 	0
#define EEPROM_P1_ENG_ADDR		8
#define EEPROM_P2_VALUE_ADDR 	16
#define EEPROM_P2_ENG_ADDR		24

#define STR_ENDLINE 	"\r\n"

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define LED_OFF 		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET)
#define LED_ON		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET)
#define LED_TOOGLE 	HAL_GPIO_Toggle(LED_GPIO_Port, LED_Pin)
#define LKEY_READ	HAL_GPIO_ReadPin(LKEY_GPIO_Port, LKEY_Pin)
#define MKEY_READ	HAL_GPIO_ReadPin(MKEY_GPIO_Port, MKEY_Pin)
#define RKEY_READ	HAL_GPIO_ReadPin(RKEY_GPIO_Port, RKEY_Pin)

#define PRINT_EOL	printf(STR_ENDLINE)

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
HX711_HandleTypeDef scale;
HX711_ScalingTypeDef transferFn;
char numStr[STR_BUFFER_SIZE];

uint32_t buffer[BUFFER_SIZE];

uint8_t UserRxBuffer[1000];
uint8_t cmdStr[1000];
uint8_t *cmdStrPt = cmdStr;
uint8_t stringComplete = false;
uint32_t serialAvalible = 0;

uint32_t p1 = 2;
uint32_t p2 = 500;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
void serialEvent(void);
void parse_cmd(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_I2C1_Init();
	MX_USB_DEVICE_Init();
	/* USER CODE BEGIN 2 */
	ssd1306_Init();

	HX711_Init(&scale, PD_SCK_GPIO_Port, PD_SCK_Pin, DOUT_GPIO_Port, DOUT_Pin, HX711_CH_A_GAIN_128);

	transferFn.p1_value = EEPROM_24LC02B_read_uint32(EEPROM_P1_VALUE_ADDR);
	transferFn.p1_eng = EEPROM_24LC02B_read_uint32(EEPROM_P1_ENG_ADDR);
	transferFn.p2_value = EEPROM_24LC02B_read_uint32(EEPROM_P2_VALUE_ADDR);
	transferFn.p2_eng = EEPROM_24LC02B_read_uint32(EEPROM_P2_ENG_ADDR);
	HX711_UpdateScaling(&transferFn);

	ssd1306_SetCursor(2, 0);
	ssd1306_WriteString("Tare", Font_7x10, White);
	ssd1306_UpdateScreen();

	HX711_Tare(&scale, 16);
	itoa(scale.offset, numStr, 10);
	ssd1306_SetCursor(2, 0);
	ssd1306_WriteString(numStr, Font_7x10, White);
	ssd1306_UpdateScreen();

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		//serialEvent();
		parse_cmd();

		LED_ON;

		//num = HX711_Average(&scale, 4) - scale.offset;

		// Shift buffer over
		for (uint8_t i = 0; i < (BUFFER_SIZE - 1); i++) {
			buffer[i] = buffer[i + 1];
		}

		// add new value to end of buffer
		buffer[BUFFER_SIZE - 1] = HX711_Value(&scale);

		// average buffer
		uint32_t sum = 0;
		for (uint8_t i = 0; i < BUFFER_SIZE; i++) {
			sum += buffer[i];
		}

		uint32_t avg = sum / BUFFER_SIZE;

		int32_t grams = HX711_Calculate(&transferFn, avg) - HX711_Calculate(&transferFn, scale.offset);

		LED_OFF;

		ssd1306_SetCursor(2, 10);
		itoa(avg, numStr, 10);
		ssd1306_WriteString(numStr, Font_7x10, White);
		ssd1306_WriteString("    ", Font_7x10, White);

		/*		ssd1306_SetCursor(2, 10 + 10);
		 uint32_t num = avg - scale.offset;
		 itoa(num, numStr, 10);
		 ssd1306_WriteString(numStr, Font_11x18, White);
		 ssd1306_WriteString("    ", Font_11x18, White);*/

		ssd1306_SetCursor(2, 10 + 10 + 18);
		itoa(grams, numStr, 10);
		strcat(numStr, " g");
		ssd1306_WriteString(numStr, Font_11x18, White);
		ssd1306_WriteString("    ", Font_11x18, White);

		ssd1306_UpdateScreen();

		if (LKEY_READ == 0) {
			ssd1306_SetCursor(2, 0);
			ssd1306_WriteString("Tare    ", Font_7x10, White);
			ssd1306_UpdateScreen();

			HX711_Tare(&scale, 32);

			itoa(scale.offset, numStr, 10);
			ssd1306_SetCursor(2, 0);
			ssd1306_WriteString(numStr, Font_7x10, White);
			ssd1306_UpdateScreen();

			while (LKEY_READ == 0)
				;
		}

		if (MKEY_READ == 0) {
			ssd1306_SetCursor(2, 10 + 10 + 18);

			scale.gain++;

			if (scale.gain > 3) {
				scale.gain = 1;
			}

			itoa(scale.gain, numStr, 10);
			ssd1306_WriteString(numStr, Font_6x8, White);
			ssd1306_UpdateScreen();

			while (MKEY_READ == 0)
				;

		}
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

	/** Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
	PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */
PUTCHAR_PROTOTYPE {
	while (CDC_Transmit_FS((uint8_t*) &ch, 1) == USBD_BUSY)
		;

	return ch;
}

void serialEvent(void) {
	if (serialAvalible) {
		for (uint32_t i = 0; i < serialAvalible; i++) {
			if (UserRxBuffer[i] != '\r') {
				*cmdStrPt = UserRxBuffer[i];
				cmdStrPt++;
			}

			if (UserRxBuffer[i] == '\n') {
				stringComplete = true;
				cmdStrPt = cmdStr;
			}
		}
	}

	serialAvalible = 0;
}

void parse_cmd(void) {
	char *pt;

	if (stringComplete) {
		stringComplete = 0;

		if (cmdStr[0] == '?') {
			PRINT_EOL;
			printf("Possible Commands");
			PRINT_EOL;
			printf("P1? or P2? to get transfer function settings");
			PRINT_EOL;
			printf("P1=x to set value in grams");
			PRINT_EOL;
			printf("P2=x to set value in grams");
			PRINT_EOL;
		} else if ((cmdStr[0] == 'P') || (cmdStr[0] == 'p')) {
			if (cmdStr[1] == '1') {
				if (cmdStr[2] == '?') {
					printf("P1: value=%lu, eng=%lu", transferFn.p1_value, transferFn.p1_eng);
					PRINT_EOL;
				} else if (cmdStr[2] == '=') {
					pt = strchr((char*) cmdStr, '=');
					printf("P1 updating...");
					PRINT_EOL;
					transferFn.p1_eng = strtoul(pt + 1, NULL, 10);
					transferFn.p1_value = HX711_Average(&scale, 32);

					EEPROM_24LC02B_write_uint32(EEPROM_P1_ENG_ADDR, &transferFn.p1_eng);
					EEPROM_24LC02B_write_uint32(EEPROM_P1_VALUE_ADDR, &transferFn.p1_value);

					HX711_UpdateScaling(&transferFn);

					printf("\tvalue=%lu", transferFn.p1_value);
					PRINT_EOL;
					printf("\teng=%lu", transferFn.p1_eng);
					PRINT_EOL;
				} else {
					printf("invalid command!%s", STR_ENDLINE);
				}
			} else if (cmdStr[1] == '2') {
				if (cmdStr[2] == '?') {
					printf("P2: value=%lu, eng=%lu", transferFn.p2_value, transferFn.p2_eng);
					PRINT_EOL;
				} else if (cmdStr[2] == '=') {
					pt = strchr((char*) cmdStr, '=');
					printf("P2 updating...");
					PRINT_EOL;
					transferFn.p2_eng = strtoul(pt + 1, NULL, 10);
					transferFn.p2_value = HX711_Average(&scale, 32);

					EEPROM_24LC02B_write_uint32(EEPROM_P2_ENG_ADDR, &transferFn.p2_eng);
					EEPROM_24LC02B_write_uint32(EEPROM_P2_VALUE_ADDR, &transferFn.p2_value);

					HX711_UpdateScaling(&transferFn);

					printf("\tvalue=%lu", transferFn.p2_value);
					PRINT_EOL;
					printf("\teng=%lu", transferFn.p2_eng);
					PRINT_EOL;
				} else {
					printf("invalid command!%s", STR_ENDLINE);
				}
			}
		} else {
			printf("type ? for help\n\r");
		}
	}
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
