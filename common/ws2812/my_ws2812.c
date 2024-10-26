
#include "my_ws2812.h"
static const char *TAG = "WS2812";

#define EXAMPLE_CHASE_SPEED_MS (10)

led_strip_t* ws2812_init( int gpio_num, int rmt_tx_channel, int led_number)
{
    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(gpio_num, rmt_tx_channel);
    // set counter clock to 40MHz
    config.clk_div = 2;

    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));

    // install ws2812 driver
    led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(led_number, (led_strip_dev_t)config.channel);
    led_strip_t* strip = led_strip_new_rmt_ws2812(&strip_config);
    if (!strip) {
        ESP_LOGE(TAG, "install WS2812 driver failed");
    }
    // Clear LED strip (turn off all LEDs)
    ESP_ERROR_CHECK(strip->clear(strip, 100));
    return strip;
}

void ws2812_set_color(led_strip_t *strip, int index, int red, int green, int blue)
{
    ESP_ERROR_CHECK(strip->set_pixel(strip, index, red, green, blue));
    //ESP_ERROR_CHECK(strip->refresh(strip, 100));
}