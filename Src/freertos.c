/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "icm20948_lib.h"
#include "i2c.h"
#include "mqtt.h"
#include "VEML3328.h"
#include "printf.h"
#include "math.h"
#include <stdio.h>
#include <string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define VEML_SPI hi2c1
#define ACCEL 0
#define GYRO 1
#define ANGLE 2
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
osEventFlagsId_t mqttflags;

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for VEML */
osThreadId_t VEMLHandle;
const osThreadAttr_t VEML_attributes = {
  .name = "VEML",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for ICM */
osThreadId_t ICMHandle;
const osThreadAttr_t ICM_attributes = {
  .name = "ICM",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for mqttTask */
osThreadId_t mqttTaskHandle;
const osThreadAttr_t mqttTask_attributes = {
  .name = "mqttTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void veml_task(void *argument);
void icm_task(void *argument);
void mqttEntry(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
	mqttflags = osEventFlagsNew(NULL);
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of VEML */
  VEMLHandle = osThreadNew(veml_task, NULL, &VEML_attributes);

  /* creation of ICM */
  ICMHandle = osThreadNew(icm_task, NULL, &ICM_attributes);

  /* creation of mqttTask */
  mqttTaskHandle = osThreadNew(mqttEntry, NULL, &mqttTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
	  osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_veml_task */
/**
* @brief Function implementing the VEML thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_veml_task */
void veml_task(void *argument)
{
  /* USER CODE BEGIN veml_task */
	osEventFlagsWait(mqttflags, 0x01, osFlagsWaitAny, osWaitForever);
	char topic[20] = "fh/project/color";
	char payload[50];
	char *p_str = &payload[0];
	mqtt_msg_t message;


	uint8_t r_raw[2];
	uint8_t g_raw[2];
	uint8_t b_raw[2];
	uint16_t data[3];
	veml_init(&VEML_SPI);
  /* Infinite loop */
  for(;;)
  {
	veml_read(CMD_RED,r_raw);
	veml_read(CMD_GREEN,g_raw);
	veml_read(CMD_BLUE,b_raw);

	data[R] = (r_raw[MSB] << 8) | r_raw[LSB];
	data[G] = (g_raw[MSB] << 8) | g_raw[LSB];
	data[B] = (b_raw[MSB] << 8) | b_raw[LSB];

	sprintf(p_str,"%+0.4hX:%+0.4hX:%+0.4hX",data[R],data[G],data[B]);
	message.topic = topic;
	message.payload = payload;
	message.payload_length = sizeof(payload);

	mqtt_publish(message, 250);



    osDelay(1000);
  }
  /* USER CODE END veml_task */
}

/* USER CODE BEGIN Header_icm_task */
/**
* @brief Function implementing the ICM thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_icm_task */
void icm_task(void *argument)
{
  /* USER CODE BEGIN icm_task */
	osEventFlagsWait(mqttflags, 0x01, osFlagsWaitAny, osWaitForever);
	char topic_accel[20] = "fh/project/accel";
	char topic_gyro[20] = "fh/project/gyro";
	char topic_angle[20] = "fh/project/angle";
	char payload[50];
	char *p_str = &payload[0];
	mqtt_msg_t message;

	axises my_accel;
	axises my_gyro;
	float x[3] = {0};
	float y[3] = {0};
	float z[3] = {0};
	icm20948_init();
  /* Infinite loop */
  for(;;)
  {
	  icm20948_accel_read(&my_accel);
	  x[ACCEL] = (float)my_accel.x/pow(2, 14);
	  y[ACCEL] = (float)my_accel.y/pow(2, 14);
	  z[ACCEL] = (float)my_accel.z/pow(2, 14);

	  sprintf(p_str, "%.2f:%.2f:%.2f",x[ACCEL],y[ACCEL],z[ACCEL]);
	  message.topic = topic_accel;
	  message.payload = payload;
	  message.payload_length = sizeof(payload);

	  mqtt_publish(message, 250);
	  //printf("MSG: %s\n", message.payload);

//	  icm20948_gyro_read(&my_gyro);
//	  x[GYRO] = (float)my_gyro.x/pow(2, 7);
//	  y[GYRO] = (float)my_gyro.y/pow(2, 7);
//	  z[GYRO] = (float)my_gyro.z/pow(2, 7);
//
//	  sprintf(p_str, "%.2f:%.2f:%.2f",x[GYRO],y[GYRO],z[GYRO]);
//	  message.topic = topic_gyro;
//	  message.payload = payload;
//	  message.payload_length = sizeof(payload);
//
//	  mqtt_publish(message, 250);
//	  //printf("MSG: %s\n", message.payload);

	  x[ANGLE] = atan(x[ACCEL] / sqrt(y[ACCEL] * y[ACCEL] + z[ACCEL] * z[ACCEL])) * (180 / M_PI);	// Pitch
	  y[ANGLE] = atan(y[ACCEL] / sqrt(x[ACCEL] * x[ACCEL] + z[ACCEL] * z[ACCEL])) * (180 / M_PI);  // Roll

	  sprintf(p_str, "%.2f:%.2f:%.2f",x[ANGLE],y[ANGLE],z[ANGLE]);
	  message.topic = topic_angle;
	  message.payload = payload;
	  message.payload_length = sizeof(payload);
	  //printf("MSG: %s\n", message.payload);
	  mqtt_publish(message, 250);

    osDelay(1000);
  }
  /* USER CODE END icm_task */
}

/* USER CODE BEGIN Header_mqttEntry */
/**
* @brief Function implementing the mqttTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_mqttEntry */
void mqttEntry(void *argument)
{
  /* USER CODE BEGIN mqttEntry */
	mqtt_status_t status = mqtt_init();
	printf("Init return: %u\n", status);
	if (status == MQTT_OK) {
		osEventFlagsSet(mqttflags, 0x01);
	}
  /* Infinite loop */
  for(;;)
  {
    osDelay(1000);
  }
  /* USER CODE END mqttEntry */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

