/*
 * 24LC02B.h
 *
 *  Created on: Mar 6, 2020
 *      Author: samper
 */

#ifndef INC_EEPROM_24LC02B_H_
#define INC_EEPROM_24LC02B_H_

#include "main.h"
#include <string.h>
#include <stdlib.h>

#define EEPROM_24LC02B_DEVICE_ADDR 	(0x50 << 1)
#define EEPROM_24LC02B_MAX_ADDR		0xFF
#define EEPROM_24LC02B_PAGE_SIZE	8

#define EEPROM_24LC02B_OK 		0
#define EEPROM_24LC02B_NOUP		1
#define EEPROM_24LC02B_ERROR 	2

//typedef struct _eeprom_24lc02b_DataTypeDef_uint8 {
//	uint8_t addr;
//	uint8_t val;
//
//} EEPROM_24LC02B_DataTypeDef_uint8;

// Change this to match the i2c used
#define EEPROM_24LC02B_I2C_PORT hi2c1
extern I2C_HandleTypeDef EEPROM_24LC02B_I2C_PORT;

// Read data function prototypes
uint8_t EEPROM_24LC02B_read_uint8(uint8_t data_addr);
int8_t EEPROM_24LC02B_read_int8(uint8_t data_addr);
uint16_t EEPROM_24LC02B_read_uint16(uint8_t data_addr);
int16_t EEPROM_24LC02B_read_int16(uint8_t data_addr);
uint32_t EEPROM_24LC02B_read_uint32(uint8_t data_addr);
int32_t EEPROM_24LC02B_read_int32(uint8_t data_addr);
float EEPROM_24LC02B_read_float(uint8_t data_addr);
double EEPROM_24LC02B_read_double(uint8_t data_addr);

// Write data function prototypes
uint8_t EEPROM_24LC02B_write_uint8(uint8_t data_addr, uint8_t *pData);
uint8_t EEPROM_24LC02B_write_int8(uint8_t data_addr, int8_t *pData);
uint8_t EEPROM_24LC02B_write_uint16(uint8_t data_addr, uint16_t *pData);
uint8_t EEPROM_24LC02B_write_int16(uint8_t data_addr, int16_t *pData);
uint8_t EEPROM_24LC02B_write_uint32(uint8_t data_addr, uint32_t *pData);
uint8_t EEPROM_24LC02B_write_int32(uint8_t data_addr, int32_t *pData);
uint8_t EEPROM_24LC02B_write_float(uint8_t data_addr, float *pData);
uint8_t EEPROM_24LC02B_write_double(uint8_t data_addr, double *pData);

void EEPROM_24LC02B_memdump(uint8_t *pData, uint8_t len);

// char string data read and write
void EEPROM_24LC02B_read_str(uint8_t data_addr, uint8_t *str, uint8_t len);
uint8_t EEPROM_24LC02B_write_str(uint8_t data_addr, uint8_t *str, uint8_t len);

#endif /* INC_EEPROM_24LC02B_H_ */
