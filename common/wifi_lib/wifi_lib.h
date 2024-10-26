#ifndef WIFI_LIB_H
#define WIFI_LIB_H
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
void wifi_setup (uint8_t* your_ssid, uint8_t* your_password);
void wifi_init_sta(void);
void change_wifi_info(char *ur_ssid, char*ur_password);
#endif