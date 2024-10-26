#ifndef WIFI_CONNECT_H
#define WIFI_CONNECT_H

typedef enum
{
    access_point,
    smart_config
}connect_type_t;

typedef void (*event_callback)(void);
void change_wifi_info(char *ur_ssid, char*ur_password);
void smartconfig_wifi_connect();
void set_wifi_sta_connected_callback( void *cb);
#endif