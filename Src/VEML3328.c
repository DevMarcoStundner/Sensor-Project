/*
 * VEML3328.c
 *
 *  Created on: Jun 12, 2023
 *      Author: marco
 */

#include "VEML3328.h"


const static uint8_t VEML_ADDW = 0x10 << 1;
const static uint8_t VEML_ADDR = VEML_ADDW | 1;

static I2C_HandleTypeDef * veml_i2c = NULL;


/** brief Function veml_read() will write to setup the veml
 */
HAL_StatusTypeDef veml_write()
{
	uint8_t txdata[2];
	txdata[0] = 0x00;
	txdata[1] = 0x00 >> 8;
	return HAL_I2C_Mem_Write(veml_i2c, VEML_ADDR, VEML_ADDW, I2C_MEMADD_SIZE_8BIT, txdata, 2, 100);
}

/** brief Function veml_read() will read the raw data from the specified Channel
 *  param cmd is the Commandword that should be read from
 *  param data is the Array were the data should be safed in
 *  return HAL Errorcode
 */
HAL_StatusTypeDef veml_read(veml_cmd_t cmd, uint8_t data[2])
{
	return HAL_I2C_Mem_Read(veml_i2c, VEML_ADDR, cmd, I2C_MEMADD_SIZE_8BIT, data, 2, 100);
}

/** brief Function veml_init() will init and set up the VEML3328
 *  param hi2c is the Handle for the I2C
 *  return HAL Errorcode
 */
HAL_StatusTypeDef veml_init(I2C_HandleTypeDef *hi2c)
{
	veml_i2c = hi2c;

	if(veml_write() != HAL_OK)
		return HAL_ERROR;
	return HAL_OK;
}

