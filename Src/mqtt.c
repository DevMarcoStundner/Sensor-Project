#include "mqtt.h"

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "cmsis_os2.h"
#include "stm32l4xx_hal.h"
#include "gpio.h"

#define RXBUFSIZE (128)
#define RXBUFNUM (10)
#define RXMATCHC ((uint8_t)'\n')

#define CMDBUFSIZE (128)

#define OSTXFINOKFLAG (0x01)
#define OSTXFINERRFLAG (0x02)
#define OSTXFINFLAGS (0x3)
#define OSFLAGSERR (0x80000000)

typedef struct {
  char buf[RXBUFSIZE+1];
  bool filled;
} rxbuf_t;

UART_HandleTypeDef * mqtt_uart = &huart1; // change to correct uart

// private mqtt settings
static char * wifi_name = "Marcos iPhone (2)";
static char * wifi_key = "12345678";
static char * broker_address = "xivps.xyz";
static char * broker_port = "8883";
static char * broker_user = "fhembsys";
static char * broker_password = "12345";
static char * broker_id = "sensor";

static rxbuf_t rxbuffers[RXBUFNUM];

static uint32_t rxtimeout = 100;

static osEventFlagsId_t evttxflags = NULL;
static osSemaphoreId_t semtx = NULL, semalock = NULL, semrx;

// send command to esp, block until response is received
static mqtt_status_t send_command(uint8_t len, char * cmd, uint32_t timeout);
static mqtt_status_t parse_buffer(mqtt_msg_t * msg, rxbuf_t * buffer);

void rxbuf_switch() {
  static rxbuf_t * activebuf = rxbuffers;
  rxbuf_t * prevbuf = activebuf;
  HAL_UART_DMAStop(mqtt_uart);
  for (int i=0; i<RXBUFNUM; i++) {
    if ((rxbuffers[i].filled == false) && (rxbuffers+i != activebuf)) {
      activebuf = rxbuffers+i;
      break;
    }
  }
  HAL_UART_Receive_DMA(mqtt_uart, (uint8_t*)activebuf->buf, RXBUFSIZE);

  if (activebuf != prevbuf) { // only use buffer if a new buffer has been found before
    if (prevbuf->buf[0] == '+') {
      prevbuf->filled = true; // only activate buffer if valid AT message
    } else if (0 == strncmp(prevbuf->buf, "OK", 2)) {
      osEventFlagsSet(evttxflags, OSTXFINOKFLAG);
    } else if (0 == strncmp(prevbuf->buf, "ERROR", 5)) {
      osEventFlagsSet(evttxflags, OSTXFINERRFLAG);
    }
  }
};

static void _mqtt_release_semtx() {
  osSemaphoreRelease(semtx);
}

static mqtt_status_t send_command(uint8_t len, char * cmd, uint32_t timeout) {
  if (len == 0 || cmd == NULL) {
    return MQTT_PARAM;
  }
  osStatus_t semret __attribute__((cleanup(_mqtt_release_semtx))) = osSemaphoreAcquire(semtx, timeout);
  switch (semret) {
    case osErrorTimeout:
      return MQTT_TIMEOUT;
    case osOK:
      break; // dont do anything, queens of the stone age
    default:
      return MQTT_ERR;
  }
  if (HAL_UART_Transmit_DMA(mqtt_uart, (uint8_t *)cmd, len) != HAL_OK) {
    return MQTT_ERR;
  }
  uint32_t evtret = osEventFlagsWait(evttxflags, OSTXFINFLAGS, osFlagsWaitAny, rxtimeout);
  if (evtret & OSFLAGSERR) {
    return MQTT_TIMEOUT;
  } else {
    if (evtret & OSTXFINOKFLAG) {
      return MQTT_OK;
    } else {
      return MQTT_ERR;
    }
  }
}

mqtt_status_t mqtt_init() {
  // TODO add error handling here
  semalock = osSemaphoreNew(1,1,NULL);
  semtx = osSemaphoreNew(1, 1, NULL); // init internal tx semaphore
  semrx = osSemaphoreNew(1, 1, NULL); // init internal tx semaphore
  evttxflags = osEventFlagsNew(NULL);

  for(int i=0; i<RXBUFNUM; i++) {
    rxbuffers[i].buf[RXBUFSIZE] = '\0';
  }

  //HAL_UART_Transmit(mqtt_uart, "AT", 2, 100); // just for basic debugging

  // enable usart character match pattern
  __HAL_UART_DISABLE(mqtt_uart);
  USART1->CR2 |= ('\n'<<24) | USART_CR2_ADDM7;
  __HAL_UART_ENABLE(mqtt_uart);

  HAL_UART_RegisterCallback(mqtt_uart, HAL_UART_RX_COMPLETE_CB_ID, rxbuf_switch);
  __HAL_UART_ENABLE_IT(mqtt_uart, UART_IT_CM);
  rxbuf_switch();
  //HAL_UART_Receive_DMA(mqtt_uart, (uint8_t*)rxbuffers->buf, RXBUFSIZE);

  static char cmdbuf[CMDBUFSIZE] = "";
  int ret = -1;

  // ATE -> check that the device is up and turn echoing off
  ret = snprintf(cmdbuf, CMDBUFSIZE, "ATE0\r\n");
  if (ret <= 0) {
    return MQTT_ERR;
  }
  rxtimeout = 1000;
  uint8_t cnt = 0;
  while (1) {
    if (send_command(ret, cmdbuf, 0) == MQTT_OK) {
      break;
    }
    if (cnt > 5) { // number of retries is variable
      return MQTT_ERR;
    } else {
      cnt++;
    }
  }
  rxtimeout = 20000;

  // AT+SYSLOG=1 -> emable error code printout, only used for logic analyzer
  ret = snprintf(cmdbuf, CMDBUFSIZE, "AT+SYSLOG=1\r\n");
  if (ret <= 0) {
    return MQTT_ERR;
  }
  if (send_command(ret, cmdbuf, 0) != MQTT_OK) {
    return MQTT_ERR;
  }

  // AT+GMR -> list version, only readable by logic analyzer
  ret = snprintf(cmdbuf, CMDBUFSIZE, "AT+GMR\r\n");
  if (ret <= 0) {
    return MQTT_ERR;
  }
  if (send_command(ret, cmdbuf, 0) != MQTT_OK) {
    return MQTT_ERR;
  }

  // AT+MQTTCLEAN=0 -> reset mqtt connection in case stm is reset but esp not
  ret = snprintf(cmdbuf, CMDBUFSIZE, "AT+MQTTCLEAN=0\r\n");
  if (ret <= 0) {
    return MQTT_ERR;
  }
  send_command(ret, cmdbuf, 0); // will cause error when properly reset, but we dont care about that

  // AT+CWMODE=1 -> enable wifi client mode
  ret = snprintf(cmdbuf, CMDBUFSIZE, "AT+CWMODE=1\r\n");
  if (ret <= 0) {
    return MQTT_ERR;
  }
  if (send_command(ret, cmdbuf, 0) != MQTT_OK) {
    return MQTT_ERR;
  }

  // AT+CWJAP="<ssid>","<key>" -> connect to wifi
  ret = snprintf(cmdbuf, CMDBUFSIZE, "AT+CWJAP=\"%s\",\"%s\"\r\n", wifi_name, wifi_key);
  if (ret <= 0) {
    return MQTT_ERR;
  }
  if (send_command(ret, cmdbuf, 0) != MQTT_OK) {
    return MQTT_ERR;
  }

  // AT+MQTTUSERCFG=0,2,"<id>","<user>","dummypw",0,0,""
  ret = snprintf(cmdbuf, CMDBUFSIZE, "AT+MQTTUSERCFG=0,2,\"%s\",\"%s\",\"dummypw\",0,0,\"\"\r\n", broker_id, broker_user);
  if (ret <= 0) {
    return MQTT_ERR;
  }
  if (send_command(ret, cmdbuf, 0) != MQTT_OK) {
    return MQTT_ERR;
  }

  // AT+MQTTPASSWORD=0,"<passwd>"
  ret = snprintf(cmdbuf, CMDBUFSIZE, "AT+MQTTPASSWORD=0,\"%s\"\r\n", broker_password);
  if (ret <= 0) {
    return MQTT_ERR;
  }
  if (send_command(ret, cmdbuf, 0) != MQTT_OK) {
    return MQTT_ERR;
  }

  // AT+MQTTCONN=0,"<addr>",port,1
  ret = snprintf(cmdbuf, CMDBUFSIZE, "AT+MQTTCONN=0,\"%s\",%s,1\r\n", broker_address, broker_port);
  if (ret <= 0) {
    return MQTT_ERR;
  }
  if (send_command(ret, cmdbuf, 0) != MQTT_OK) {
    return MQTT_ERR;
  }

  rxtimeout = 1000;
  mqtt_msg_t message;
  message.topic = "fh/project/status";
  message.payload = "sensor online";
  mqtt_publish(message, 1000);

  return MQTT_OK;
}

// subscribe to mqtt topic
mqtt_status_t mqtt_subscribe(const char * topic) {
  static char cmdbuf[CMDBUFSIZE] = "";
  int ret = -1;
  // AT+MQTTSUB=0,"<topic>",0
  ret = snprintf(cmdbuf, CMDBUFSIZE, "AT+MQTTSUB=0,\"%s\",2\r\n", topic);
  if (ret <= 0) {
    return MQTT_ERR;
  }
  if (send_command(ret, cmdbuf, 0) != MQTT_OK) {
    return MQTT_ERR;
  }

  return MQTT_OK;
}

mqtt_status_t mqtt_publish(mqtt_msg_t message, uint32_t timeout) {
  static char cmdbuf[CMDBUFSIZE] = "";
  int ret = -1;
  // AT+MQTTPUB=0,"<topic>","<value>",0,0
  ret = snprintf(cmdbuf, CMDBUFSIZE, "AT+MQTTPUB=0,\"%s\",\"%s\",0,0\r\n", message.topic, message.payload);
  if (ret <= 0) {
    return MQTT_ERR;
  }
  return send_command(ret, cmdbuf, timeout);
}

static mqtt_status_t parse_buffer(mqtt_msg_t * msg, rxbuf_t * buffer) {
  msg->topic = NULL;
  msg->payload = NULL;
  msg->payload_length = 0;
  if (0 == strncmp(buffer->buf, "+MQTTSUBRECV", 12)) {
    buffer->buf[strchr(buffer->buf,'\n')-buffer->buf] = '\0'; // when detected as character match, a \n char is guaranteed
    uint8_t len = 0;
    char * tmpstr = strchr(buffer->buf, ',')+2; // fist part of the string, we can discard that
    // extract topic
    len = strchr(tmpstr, ',')-tmpstr;
    msg->topic = safe_malloc(len);
    if (msg->topic == NULL) {
      return MQTT_ERR;
    }
    memcpy(msg->topic, tmpstr, len-1); // minus 1 to get rid of right quote
    msg->topic[len-1] = '\0';
    // extract data length
    tmpstr = strchr(tmpstr, ',')+1; // data length
    msg->payload_length = strtoul(tmpstr, NULL, 10);
    if (msg->payload_length > 128) {
      return MQTT_ERR;
    }
    // extract data
    tmpstr = strchr(tmpstr, ',')+1; // actual data string
    len = msg->payload_length+1;
    msg->payload = safe_malloc(len);
    if (msg->payload == NULL) {
      return MQTT_ERR;
    }
    memcpy(msg->payload, tmpstr, len-1);
    msg->payload[len-1] = '\0';
    return MQTT_OK;
  } else {
    return MQTT_INVALID;
  }
}

static void _mqtt_release_semrx() {
  osSemaphoreRelease(semrx);
}

mqtt_status_t mqtt_receive(mqtt_msg_t * message, uint32_t timeout) {
  if (message == NULL) {
    return MQTT_ERR;
  }
  rxbuf_t * buffer = NULL;
  mqtt_status_t parseret = MQTT_ERR;
  osStatus_t semret __attribute__((cleanup(_mqtt_release_semrx))) = osSemaphoreAcquire(semrx, timeout);
  if (semret == osOK) {
    for (int i=0; i<RXBUFNUM; i++) {
      buffer = rxbuffers+i;
      if (buffer->filled) {
        parseret = parse_buffer(message, buffer);
        buffer->filled = false;
        switch (parseret) {
          case MQTT_OK:
            return MQTT_OK;
          case MQTT_INVALID:
            return MQTT_EMPTY;
          default:
            safe_free(message->topic);
            safe_free(message->payload);
            return MQTT_ERR;
        }
      }
    }
    return MQTT_EMPTY; // return error in case no message is available
  } else if (semret == osErrorTimeout || semret == osErrorResource) {
    return MQTT_TIMEOUT;
  }
  return MQTT_ERR;
}

void safe_free(void * ptr) {
  osSemaphoreAcquire(semalock, osWaitForever);
  free(ptr);
  osSemaphoreRelease(semalock);
}

void * safe_malloc(size_t size) {
  void * res = NULL;
  osSemaphoreAcquire(semalock, osWaitForever);
  res = malloc(size);
  osSemaphoreRelease(semalock);
  return res;
}
