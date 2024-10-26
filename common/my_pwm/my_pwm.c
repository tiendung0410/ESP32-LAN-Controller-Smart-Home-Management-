#include "my_pwm.h"
void timer_config(ledc_timer_config_t* ledc_timer, ledc_timer_t  timer_num , ledc_timer_bit_t reso, uint32_t freg)
{
        ledc_timer->duty_resolution = reso, // resolution of PWM duty
        ledc_timer->freq_hz = freg,                      // frequency of PWM signal
        ledc_timer->speed_mode = LEDC_HIGH_SPEED_MODE,           // timer mode
        ledc_timer->timer_num = timer_num,            // timer index
        ledc_timer->clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
        ledc_timer_config(ledc_timer);
}

void pwm_config (ledc_channel_config_t* ledc_channel_cf, ledc_timer_config_t* ledc_timer, ledc_channel_t channel,int gpio_num )
{
/*
     * Prepare and set configuration of timers
     * that will be used by LED Controller
     */
    
    // Set configuration of timer0 for high speed channels

    ledc_channel_cf->channel    = channel,
    ledc_channel_cf->duty       = 0,
    ledc_channel_cf->gpio_num   = gpio_num,
    ledc_channel_cf->speed_mode = LEDC_HIGH_SPEED_MODE,
    ledc_channel_cf->hpoint     = 0,
    ledc_channel_cf->timer_sel  = ledc_timer->timer_num;
    // Set LED Controller with previously prepared configuration
    ledc_channel_config(ledc_channel_cf);
    ledc_fade_func_install(0);
}
    
void pwm_set_duty(ledc_channel_config_t* ledc_channel_config, int duty)
{
 
    ledc_set_duty(ledc_channel_config->speed_mode, ledc_channel_config->channel, duty);
    ledc_update_duty(ledc_channel_config->speed_mode, ledc_channel_config->channel);
}
    


