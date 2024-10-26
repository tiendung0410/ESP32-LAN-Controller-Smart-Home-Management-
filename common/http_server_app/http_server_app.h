#ifndef HTTP_SERVER_APP_H
#define HTTP_SERVER_APP_H

typedef void (*func_get_callback)(void);
typedef void (*func_post_callback)(char* data, int len);
typedef void (*func_get_contain_data_callback)(char* data, int len);

void stop_webserver(void );
void start_webserver(void);
void set_switch_state_callback(void*cb);
void set_get_dht11_data_callback(void*cb);
void set_drag_slider_callback(void*cb);
void set_wifi_info_callback(void*cb);
void httpd_respone_send(const char *buf, int buf_len);
void set_rgb_callback(void*cb);




#endif
