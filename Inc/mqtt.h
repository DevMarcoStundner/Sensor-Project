#ifndef MQTT_H
#define MQTT_H

#include <stdint.h>
#include <stddef.h>

#include "usart.h"

// mqtt packet qos setting
typedef enum {
  MQTT_QOS_LEQONCE = 0, // at most once
  MQTT_QOS_GEQONCE = 1, // at least once
  MQTT_QOS_XEQONCE = 2, // exactly once
} mqtt_qos_t;

// possible return values from mqtt functions
typedef enum {
  MQTT_OK = 0,      // function returned successfully
  MQTT_ERR,     // function encountered generic error
  MQTT_DIRTY,   // function encountered error and left esp in dirty state
  MQTT_TIMEOUT, // function encountered timeout
  MQTT_PARAM,   // function encountered parameter error
  MQTT_EMPTY,   // function was not able to run because of missing resources
  MQTT_INVALID, // function has no way to handle given data
} mqtt_status_t;

// packet type for mqtt messages, used both for tx and rx queues
typedef struct {
  char * topic; // null terminated topic string, maximum length 128
  char * payload; // payload string
  uint32_t payload_length; // payload length in bytes, MQTT allows for a maximum of 2*32-1 bytes per message
  mqtt_qos_t qos;
} mqtt_msg_t;

extern UART_HandleTypeDef * mqtt_uart;

// initialize the mqtt library:
// - populate queues
// - configure esp32 wifi
// - connect to mqtt broker
mqtt_status_t mqtt_init();

void * safe_malloc(size_t size);
void safe_free(void * ptr);

// just here so the compiler finds the function for the interrupt
void rxbuf_switch();

// publish mqtt message, this works via a semaphore
// note that the timeout only handles the waiting period in case of multiple
// waiting transmissions, once the transmission has started an additional
// maximum 10 seconds may flow until the function returns.
mqtt_status_t mqtt_publish(mqtt_msg_t message, uint32_t timeout);

// receive one mqtt message if available, wait for timeout if none available.
mqtt_status_t mqtt_receive(mqtt_msg_t * message, uint32_t timeout);

// subscribe to mqtt topic
mqtt_status_t mqtt_subscribe(const char * topic);

#endif
