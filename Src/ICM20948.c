/*
 * ICM20948.c
 *
 *  Created on: Jun 15, 2023
 *      Author: marco
 */

#include "ICM20948.h"


static SPI_HandleTypeDef * icm_spi = NULL;

/** brief Function icm_write() will write data to the given address
 * 	parameter cmd takes Address for the icm register
 * 	parameter data takes Data that should be written to the icm register
 *  return HAL Errorcode
 */
HAL_StatusTypeDef icm_write(icm_cmd_t cmd, icm_value_t data)
{
	uint8_t txdata[2];
	txdata[0] = ((uint8_t)cmd);
	txdata[1] = (uint8_t)data;

	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);
	if(HAL_SPI_Transmit(icm_spi, txdata, 2, 100) == HAL_OK)
	{
		HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET);
		return HAL_OK;
	}
	else
	{
		HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET);
		return HAL_ERROR;
	}
}

/** brief Function icm_read() will read data from the given address
 * 	parameter cmd takes Address for the icm register
 * 	parameter data stores Data from the icm register
 *  return HAL Errorcode
 */
HAL_StatusTypeDef icm_read(icm_cmd_t cmd, uint8_t data)
{
	uint8_t rxdata[2];
	uint8_t txdata[2];

	rxdata[0] = ((uint8_t)cmd) | Read << MSB_;
	rxdata[1] = 0;



	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);
	if(HAL_SPI_TransmitReceive(icm_spi, rxdata, txdata, 2, 100) == HAL_OK)
	{
		HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET);
		return HAL_OK;
	}
	else
	{
		HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET);
		return HAL_ERROR;
	}
}

/** brief Function icm_init() will write to setup the icm
 *  return HAL Errorcode
 */
HAL_StatusTypeDef icm_init(SPI_HandleTypeDef *hspi)
{

	icm_spi = hspi;

	if(icm_write(CMD_REG_BANK_SEL, VAL_REG_BANK_SEL_0) == HAL_OK)
	{
		icm_write(CMD_USER_CTRL, VAL_USER_CTRL);
		icm_write(CMD_LP_CONFIG, VAL_LP_CONFIG);
		icm_write(CMD_PWR_MGMT_1, VAL_PWR_MGMT_1);
		icm_write(CMD_PWR_MGTM_2, VAL_PWR_MGMT_2);
	}
	else
	{
		return HAL_ERROR;
	}
	if(icm_write(CMD_REG_BANK_SEL, VAL_REG_BANK_SEL_2) == HAL_OK)
	{
		icm_write(CMD_GYRO_SMPLRT_DIV, VAL_GYRO_SMPLRT_DIV);
		icm_write(CMD_GYRO_CONFIG, VAL_GYRO_CONFIG);
		icm_write(CMD_ODR_ALIGN_EN, VAL_ODR_ALIGN_EN);

		icm_write(CMD_ACCEL_SMPLRT_DIV_1, VAL_ACCEL_SMPLRT_DIV_1);
		icm_write(CMD_ACCEL_SMPLRT_DIV_2, VAL_ACCEL_SMPLRT_DIV_2);
		icm_write(CMD_ACCEL_INTEL_CTRL, VAL_ACCEL_INTEL_CTRL);
		icm_write(CMD_ACCEL_CONFIG, VAL_ACCEL_CONFIG);
	}
	else
	{
		return HAL_ERROR;
	}

	return HAL_OK;
}
