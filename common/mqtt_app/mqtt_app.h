#ifndef MQTT_APP_H
#define MQTT_APP_H
#include "mqtt_client.h"
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"



ESP_EVENT_DECLARE_BASE(MY_MQTT_EVENT);

typedef enum{
    MY_MQTT_EVENT_CONNECTED,
    MY_MQTT_EVENT_DISCONNECTED,
    MY_MQTT_EVENT_SUBSCRIBED,
    MY_MQTT_EVENT_UNSUBSCRIBED,
    MY_MQTT_EVENT_PUBLISHED,
    MY_MQTT_EVENT_DATA,
}MY_MQTT_EVENT_ID;


void mqtt_app_start(void);
// void set_mqtt_data_handler(void *cb);

#endif