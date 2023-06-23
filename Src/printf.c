/*
 * printf.c
 *
 *  Created on: Feb 21, 2023
 *      Author: marco
 */

#include "stdio.h"
#include "main.h"
#include "printf.h"
#include "stm32l4xx_hal_def.h"
#include "stm32l4xx_hal_uart.h"

#ifdef DEBUG_MODE

	extern UART_HandleTypeDef huart2;

	PUTCHAR_PROTOTYPE {
		HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
		return ch;
	}

#endif
