#ifndef MY_WS2812_H
#define MY_WS2812_H

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt.h"
#include "my_led_strip.h"
led_strip_t* ws2812_init( int gpio_num, int rmt_tx_channel, int led_number);
void ws2812_set_color(led_strip_t *strip, int index, int red, int green, int blue);
#endif