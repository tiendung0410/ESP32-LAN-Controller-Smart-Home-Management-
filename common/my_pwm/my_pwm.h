#ifndef MY_PWM_H
#define MY_PWM_H


#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"

void timer_config(ledc_timer_config_t* ledc_timer, ledc_timer_t  timer_num , ledc_timer_bit_t reso, uint32_t freg);
void pwm_config (ledc_channel_config_t* ledc_channel_config, ledc_timer_config_t* ledc_timer, ledc_channel_t channel,int gpio_num );
void pwm_set_duty(ledc_channel_config_t* ledc_channel_config, int duty);

#endif