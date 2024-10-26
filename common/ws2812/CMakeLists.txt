set(app_src my_ws2812.c)
set(pri_req led_strip)
idf_component_register(SRCS ${app_src}
                    INCLUDE_DIRS "."
                    REQUIRES ${pri_req})

