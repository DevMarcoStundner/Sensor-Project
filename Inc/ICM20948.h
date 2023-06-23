/*
 * ICM20948.h
 *
 *  Created on: Jun 15, 2023
 *      Author: marco
 */

#ifndef INC_ICM20948_H_
#define INC_ICM20948_H_

#include <stdint.h>
#include <stdio.h>
#include "main.h"
#include "printf.h"
#include "cmsis_os.h"

#define MSB_ 7
#define Write 0
#define Read 1
#define GYRO_SENS 250
#define X 0
#define Y 1
#define Z 2

typedef enum {
	//Banke 0
	CMD_WHO_AM_I = 0x00,
	CMD_USER_CTRL = 0x03,
	CMD_LP_CONFIG = 0x40,
	CMD_PWR_MGMT_1 = 0x06,
	CMD_PWR_MGTM_2 = 0x07,
	CMD_ACCEL_XOUT_H = 0x2D,
	CMD_ACCEL_XOUT_L = 0x2E,
	CMD_ACCEL_YOUT_H = 0x2F,
	CMD_ACCEL_YOUT_L = 0x30,
	CMD_ACCEL_ZOUT_H = 0x31,
	CMD_ACCEL_ZOUT_L = 0x32,
	CMD_GYRO_XOUT_H = 0x33,
	CMD_GYRO_XOUT_L = 0x34,
	CMD_GYRO_YOUT_H = 0x35,
	CMD_GYRO_YOUT_L = 0x36,
	CMD_GYRO_ZOUT_H = 0x37,
	CMD_GYRO_ZOUT_L = 0x38,
	CMD_REG_BANK_SEL = 0x7F,

	//Bank 2
	CMD_GYRO_SMPLRT_DIV = 0x00, // Div 1
	CMD_GYRO_CONFIG = 0x01,
	CMD_ODR_ALIGN_EN = 0x09,
	CMD_ACCEL_SMPLRT_DIV_1 = 0x10,
	CMD_ACCEL_SMPLRT_DIV_2 = 0x11,
	CMD_ACCEL_INTEL_CTRL = 0x12,
	CMD_ACCEL_CONFIG = 0x14

}icm_cmd_t;

typedef enum {
	//Banke 0
	VAL_USER_CTRL = 0x10,
	VAL_LP_CONFIG = 0x00,
	VAL_PWR_MGMT_1 = 0x0C,
	VAL_PWR_MGMT_2 = 0x00,
	VAL_REG_BANK_SEL_0 = 0x00,

	//Banke 2
	VAL_GYRO_SMPLRT_DIV = 0x00,
	VAL_GYRO_CONFIG = 0x00,
	VAL_ODR_ALIGN_EN = 0x01,
	VAL_ACCEL_SMPLRT_DIV_1 = 0x00,
	VAL_ACCEL_SMPLRT_DIV_2 = 0x00,
	VAL_ACCEL_INTEL_CTRL = 0x00,
	VAL_ACCEL_CONFIG = 0x00,
	VAL_REG_BANK_SEL_2 = 0x10

}icm_value_t;

HAL_StatusTypeDef icm_write(icm_cmd_t cmd, icm_value_t data);
HAL_StatusTypeDef icm_read(icm_cmd_t cmd, uint8_t data);
HAL_StatusTypeDef icm_init();

#endif /* INC_ICM20948_H_ */
