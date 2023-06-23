/*
 * VEML3328.h
 *
 *  Created on: Jun 12, 2023
 *      Author: marco
 */

#ifndef INC_VEML3328_H_
#define INC_VEML3328_H_

#include <stdint.h>
#include <stdio.h>
#include "main.h"
#include "printf.h"
#include "cmsis_os.h"

#define LSB 0
#define MSB 1
#define R 0
#define G 1
#define B 2

typedef enum {
	CMD_SETUP = 0x00,
	CMD_CLEAR = 0x04,
	CMD_RED = 0x05,
	CMD_GREEN = 0x06,
	CMD_BLUE = 0x07,
	CMD_IR = 0x08,
	CMD_ID = 0x0C
}veml_cmd_t;

HAL_StatusTypeDef veml_init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef veml_write();
HAL_StatusTypeDef veml_read(veml_cmd_t cmd, uint8_t data[2]);




#endif /* INC_VEML3328_H_ */
