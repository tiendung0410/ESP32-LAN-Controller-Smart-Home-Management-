#include "http_server_app.h"

/* Simple HTTP Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_eth.h"

#include "stdlib.h"
#include <esp_http_server.h>
#include "dht11.h"
/* A simple example that demonstrates how to create GET and POST
 * handlers for the web server.
 */

httpd_handle_t server = NULL;
static const char *TAG = "example";
struct dht11_reading dht11_data;
func_get_callback func_get_dht11_data_callack;
func_post_callback func_switch_state_callack;
func_post_callback func_drag_slider_callack;
func_post_callback func_wifi_info_callback;
func_get_contain_data_callback func_rgb_callback;
httpd_req_t *REQ;

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");
/* An HTTP GET handler */
//----------------------------------------------------------------------
static void hello_get_handler(httpd_req_t *req)
{
    /* Send response with custom headers and body set as the
     * string passed in user context*/
    // const char* resp_str = (const char*) req->user_ctx;
    // httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
    //httpd_resp_send(req, resp_str, strlen(resp_str));
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);
}
static const httpd_uri_t testserver = {
    .uri       = "/testserver",
    .method    = HTTP_GET,
    .handler   = hello_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = "Vit ngao!"
};

//---------------------------------------------------------------------------------------------------------------

static esp_err_t echo_post_handler(httpd_req_t *req)
{
    // char buf[100];
    // httpd_req_recv(req, buf,sizeof(buf));
    // //printf("%s \n",buf);

    // // End response
    // httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t echo = {
    .uri       = "/echo",
    .method    = HTTP_POST,
    .handler   = echo_post_handler,
    .user_ctx  = NULL
};

//------------------------------------------------------------------------------------------------------------------

void httpd_respone_send(const char *buf, int buf_len)
{
    httpd_resp_send(REQ,buf,buf_len);
}

static esp_err_t dht11_get_handler(httpd_req_t *req)
{
    //dht11_data= DHT11_read();
    // char data[100];
    // sprintf(data,"{\"temperature\": \"%d\", \"humidity\": \"%d\"}",dht11_data.temperature,dht11_data.humidity);
    //httpd_resp_send(req, (const char*)data, strlen(data));
    REQ=req;
    func_get_dht11_data_callack();
    return ESP_OK;
}

static const httpd_uri_t getdatadht11 = {
    .uri       = "/getdatadht11",
    .method    = HTTP_GET,
    .handler   = dht11_get_handler,
    .user_ctx  = NULL
};
//----------------------------------------------------------------------------------------------------------------
static esp_err_t rgb_get_handler(httpd_req_t *req)
{
    char*  buf;
    int buf_len;
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query => %s", buf);
            char param[32];
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "color", param, sizeof(param)) == ESP_OK) {
                func_rgb_callback(param,6);
            }
            
        }
        free(buf);
    }
    return ESP_OK;
}

static const httpd_uri_t rgb = {
    .uri       = "/rgb",
    .method    = HTTP_GET,
    .handler   = rgb_get_handler,
    .user_ctx  = NULL
};

//----------------------------------------------------------------------------------------------------------------

static esp_err_t switch_state_handler(httpd_req_t *req) {
    char buf[100];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);  // Giữ chỗ cho ký tự null
    if (ret > 0) {
        buf[ret] = '\0';  // Thêm ký tự null
        ESP_LOGI(TAG, "Data: %s", buf);
        func_switch_state_callack(buf, ret);  // Gọi callback xử lý
    } else {
        httpd_resp_send_500(req);  // Lỗi nếu không nhận được dữ liệu
        return ESP_FAIL;
    }
    // Kết thúc phản hồi
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}


static const httpd_uri_t switch1 = {
    .uri       = "/switch1",
    .method    = HTTP_POST,
    .handler   = switch_state_handler,
    .user_ctx  = NULL
};
//----------------------------------------------------------------------------------------------------------------


static esp_err_t drag_slider_handler(httpd_req_t *req)
{
    char buf[10];  // Giới hạn bộ đệm để nhận giá trị slider, chỉ cần khoảng 10 byte
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);  // Đọc dữ liệu và giữ chỗ cho null terminator
    if (ret > 0) {
        buf[ret] = '\0';  // Thêm null terminator để kết thúc chuỗi
        ESP_LOGI(TAG, "Slider Value: %s", buf);

        // Chuyển đổi dữ liệu slider thành số nguyên
        int slider_value = atoi(buf);
        if (slider_value >= 0 && slider_value <= 100) {
            func_drag_slider_callack(buf, ret);  // Gọi callback xử lý giá trị slider
        } else {
            ESP_LOGE(TAG, "Invalid slider value: %d", slider_value);
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid slider value");  // Phản hồi lỗi nếu giá trị không hợp lệ
        }
    } else {
        httpd_resp_send_500(req);  // Gửi lỗi nếu nhận dữ liệu thất bại
        return ESP_FAIL;
    }
    httpd_resp_send_chunk(req, NULL, 0);  // Kết thúc phản hồi
    return ESP_OK;
}


static const httpd_uri_t slider = {
    .uri       = "/slider",
    .method    = HTTP_POST,
    .handler   = drag_slider_handler,
    .user_ctx  = NULL
};

//----------------------------------------------------------------------------------------------------------------

static esp_err_t wifi_info_handler(httpd_req_t *req) {
    char buf[100];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);  // Giữ chỗ cho ký tự null
    if (ret > 0) {
        buf[ret] = '\0';  // Thêm ký tự null
        ESP_LOGI(TAG, "Data: %s", buf);
        func_wifi_info_callback(buf, ret);  // Gọi callback xử lý
    } else {
        httpd_resp_send_500(req);  // Lỗi nếu không nhận được dữ liệu
        return ESP_FAIL;
    }
    // Kết thúc phản hồi
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}


static const httpd_uri_t wifiinfo = {
    .uri       = "/wifiinfo",
    .method    = HTTP_POST,
    .handler   = wifi_info_handler,
    .user_ctx  = NULL
};


//----------------------------------------------------------------------------------------------------------------

static esp_err_t events_handler(httpd_req_t *req) {
    // Bạn có thể gửi sự kiện SSE tại đây nếu cần thiết
    httpd_resp_send(req, NULL, 0);  // Gửi phản hồi rỗng để giữ kết nối SSE
    return ESP_OK;
}

static const httpd_uri_t events = {
    .uri       = "/events",
    .method    = HTTP_GET,
    .handler   = events_handler,
    .user_ctx  = NULL
};
//----------------------------------------------------------------------------------------------------------------
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    if (strcmp("/testserver", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/testserver URI is not available");
        /* Return ESP_OK to keep underlying socket open */
        return ESP_OK;
    } 
    else
    {
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
    }
}

/* An HTTP PUT handler. This demonstrates realtime
 * registration and deregistration of URI handlers
 */


void start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &testserver);
        httpd_register_uri_handler(server, &echo);
        httpd_register_uri_handler(server, &switch1);
        httpd_register_uri_handler(server, &getdatadht11);
        httpd_register_uri_handler(server, &slider);
        httpd_register_uri_handler(server, &wifiinfo);
        httpd_register_uri_handler(server, &rgb);
        httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, http_404_error_handler);
        httpd_register_uri_handler(server, &events);
        DHT11_init(GPIO_NUM_10);

    }
    else{
    ESP_LOGI(TAG, "Error starting server!");
    }
}

void stop_webserver(void )
{
    // Stop the httpd server
    httpd_stop(server);

}

void set_switch_state_callback(void*cb)
{
    func_switch_state_callack=cb;
}

void set_get_dht11_data_callback(void*cb)
{
    func_get_dht11_data_callack=cb;
}

void set_drag_slider_callback(void*cb)
{
    func_drag_slider_callack=cb;
}

void set_wifi_info_callback(void*cb)
{
    func_wifi_info_callback=cb;
}

void set_rgb_callback(void*cb)
{
    func_rgb_callback=cb;
}