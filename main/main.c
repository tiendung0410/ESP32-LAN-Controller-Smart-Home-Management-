/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "http_server_app.h"
#include "wifi_connect.h"
#include "output_iot.h"
#include "dht11.h"
#include "stdlib.h"
#include "my_pwm.h"
#include "string.h"
#include "my_ws2812.h"
#include "mqtt_app.h"
#include "json_app.h"
#include "ota_app.h"

static const char *TAG = "wifi_station";
struct dht11_reading dht11_data;
ledc_timer_config_t timer1;
ledc_channel_config_t ledc_channel1;
led_strip_t* strip=NULL;
const int app_version =1;
//------------------------------------------------------------------------------------
int hexCharToInt(char c) {
    // Chuyển ký tự '0'-'9' và 'A'-'F' thành giá trị tương ứng trong hệ 16
    if (c >= '0' && c <= '9') {
        return c - '0'; // '0' tương ứng với giá trị 0, '1' với 1, ...
    } else if (c >= 'A' && c <= 'F') {
        return 10 + (c - 'A'); // 'A' tương ứng với 10, 'B' với 11, ...
    } else if (c >= 'a' && c <= 'f') {
        return 10 + (c - 'a'); // Hỗ trợ ký tự thường từ 'a'-'f'
    }
    return -1; // Nếu ký tự không hợp lệ
}

int hexStringToDecimal(const char* hexString) {
    int length = strlen(hexString); // Lấy độ dài của chuỗi
    int decimalValue = 0; // Giá trị thập phân sẽ được tính

    // Duyệt qua từng ký tự của chuỗi
    for (int i = 0; i < length; ++i) {
        int currentDigit = hexCharToInt(hexString[i]); // Chuyển ký tự sang số nguyên
        if (currentDigit == -1) {
            return -1; // Trả về -1 nếu chuỗi không hợp lệ
        }
        // Cộng giá trị số thập phân tương ứng của ký tự vào kết quả
        decimalValue = decimalValue * 16 + currentDigit;
    }

    return decimalValue;
}
//---------------------------------http request callback---------------------------------------------------


void switch_state_callback(char* data, int len)
{
   int res= atoi(data);
   output_io_set_level(GPIO_NUM_2,res);
  
}

void drag_slider_callback(char* data, int len)
{
    int res = atoi(data);  // Chuyển chuỗi nhận được từ slider thành số nguyên
    if (res >= 0 && res <= 100) {  // Kiểm tra xem giá trị nằm trong khoảng hợp lệ
        ESP_LOGI(TAG, "Setting PWM duty to: %d", res);
        pwm_set_duty(&ledc_channel1, res);  // Thiết lập giá trị PWM
    } else {
        ESP_LOGE(TAG, "Invalid slider value: %d", res);
    }
}

void wifi_info_callback(char* data, int len)
{
    char ssid[30]="";
    char password[30]="";
    char* pt;
    pt =strtok(data,"@");
    strcpy((char*)ssid,(const char*)pt);
    pt= strtok(NULL,"@");
    strcpy((char*)password,(const char*)pt);
    change_wifi_info(ssid,password);
}

void get_dht11_data_callack()
{
   dht11_data = DHT11_read();
   char data[100];
   sprintf(data,"{\"temperature\": \"%d\", \"humidity\": \"%d\"}",dht11_data.temperature,dht11_data.humidity);
   httpd_respone_send((const char*)data, strlen(data));
}

void rgb_callback(char* data, int len)
{
    printf("Color:%s\n",data);
    char r[3];
    char g[3];
    char b[3];
    strncpy(r, data, 2);   // Sao chép 2 ký tự đầu tiên
    r[2] = '\0';           // Thêm ký tự kết thúc chuỗi '\0'

    strncpy(g, data + 2, 2); // Sao chép 2 ký tự tiếp theo
    g[2] = '\0';             // Thêm ký tự kết thúc chuỗi '\0'

    strncpy(b, data + 4, 2); // Sao chép 2 ký tự cuối cùng
    b[2] = '\0';             // Thêm ký tự kết thúc chuỗi '\0'
    int red=hexStringToDecimal(r);
    int green= hexStringToDecimal(g);
    int blue=hexStringToDecimal(b);
    printf("Color:%d\n",red);
    printf("Color:%d\n",green);
    printf("Color:%d\n",blue);
    for (int i=0; i< 8;i++)
    {
       ws2812_set_color(strip,i,red,green,blue);
    }
    ESP_ERROR_CHECK(strip->refresh(strip, 100));
}
//-------------------------------------------------------

void wifi_sta_connected_callback()
{
    mqtt_app_start();
    start_webserver();
}
//----------------------------------------------------------------------
void mqtt_data_handler( char* data,int data_len)
{
    ESP_LOGE(TAG, "LOG1");
    int num_pairs;
    struct KeyValue *json_data= json_parser(data,&num_pairs);
    if (num_pairs==2)
    {
        ESP_LOGE(TAG, "LOG2");
        if (!strcmp((*json_data).key,"version") && !strcmp((*(json_data+1)).key,"url"))
        {
            if (atoi((*json_data).value) > app_version )
            {
                ota_task((*(json_data+1)).value);
            }
        }

    }
}


void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == MY_MQTT_EVENT)
    {
        if (event_id == MY_MQTT_EVENT_CONNECTED){
            ESP_LOGW(TAG, "Connected from main");
        }
        else if(event_id == MY_MQTT_EVENT_DISCONNECTED){
            ESP_LOGW(TAG, "DisConnected from main");
        }
        else if(event_id == MY_MQTT_EVENT_SUBSCRIBED){
            ESP_LOGW(TAG, "SUBSCRIBED from main");
        }
        else if(event_id == MY_MQTT_EVENT_UNSUBSCRIBED){
            ESP_LOGW(TAG, "UNSUBSCRIBED from main");
        }
        else if(event_id == MY_MQTT_EVENT_PUBLISHED){
            ESP_LOGW(TAG, "PUBLISH from main");
        }
        else if(event_id == MY_MQTT_EVENT_DATA){
            ESP_LOGW(TAG, "Data received from main");
            //esp_mqtt_event_handle_t mqtt_event = (esp_mqtt_event_handle_t) event_data;

            // Dữ liệu đã được sao chép từ event_data
            char *data = (char *) event_data;
            
            // Nếu dữ liệu hợp lệ thì xử lý
            if (data) {
                ESP_LOGI(TAG, "Data: %.*s", strlen(data), data);
                mqtt_data_handler(data, strlen(data));  // Gọi hàm xử lý dữ liệu
            }
        }
    }
}
void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    smartconfig_wifi_connect(); // connect to wifi through smart config

    //------------set callback for http request------------
    set_switch_state_callback(&switch_state_callback);
    set_get_dht11_data_callback(&get_dht11_data_callack);
    set_drag_slider_callback(&drag_slider_callback);
    set_wifi_info_callback(&wifi_info_callback);
    set_rgb_callback(&rgb_callback);
    
    output_io_create(GPIO_NUM_2);//init led to control by http

    DHT11_init(GPIO_NUM_10);// sensor to get data and push to http server

    //-----config for rgb---------------------
    timer_config(&timer1,LEDC_TIMER_0,LEDC_TIMER_7_BIT,5000);
    pwm_config(&ledc_channel1,&timer1,LEDC_CHANNEL_0,GPIO_NUM_18);
    strip=ws2812_init(GPIO_NUM_5,RMT_CHANNEL_0,8);

}
