#include "mqtt_app.h"


ESP_EVENT_DEFINE_BASE(MY_MQTT_EVENT);



static const char *TAG = "MQTTS_EXAMPLE";
void (*mqtt_data_handler_pt)(int data_len, char* data);
// extern const uint8_t client_cert_pem_start[] asm("_binary_client_crt_start");
// extern const uint8_t client_cert_pem_end[] asm("_binary_client_crt_end");
// extern const uint8_t client_key_pem_start[] asm("_binary_client_key_start");
// extern const uint8_t client_key_pem_end[] asm("_binary_client_key_end");
// extern const uint8_t mosquitto_org_crt_start[] asm("_binary_mosquitto_crt_start");
// extern const uint8_t mosquitto_org_crt_end[] asm("_binary_mosquitto_crt_end");

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_subscribe(client, "/thunderbolt/qos0", 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
            esp_event_post(MY_MQTT_EVENT,MY_MQTT_EVENT_CONNECTED,NULL,0,0);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            esp_event_post(MY_MQTT_EVENT,MY_MQTT_EVENT_DISCONNECTED,NULL,0,0);
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, "/thunderbolt/qos0", "data", 0, 0, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            esp_event_post(MY_MQTT_EVENT,MY_MQTT_EVENT_SUBSCRIBED,NULL,0,0);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            esp_event_post(MY_MQTT_EVENT,MY_MQTT_EVENT_UNSUBSCRIBED,NULL,0,0);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            esp_event_post(MY_MQTT_EVENT,MY_MQTT_EVENT_PUBLISHED,NULL,0,0);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");

            // In trực tiếp từ sự kiện MQTT
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);

            // Sao chép dữ liệu topic và data để post qua sự kiện
            char *topic = strndup(event->topic, event->topic_len);
            char *data = strndup(event->data, event->data_len);

            if (topic && data) {
                esp_event_post(MY_MQTT_EVENT, MY_MQTT_EVENT_DATA, data, event->data_len, 0);
            }

            // Giải phóng bộ nhớ đã sao chép
            free(topic);
            free(data);

            //mqtt_data_handler_pt(event->data_len,event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

void mqtt_app_start(void)
{
    const esp_mqtt_client_config_t mqtt_cfg = {
    .uri = "mqtt://test.mosquitto.org:1883",
    //.uri = "mqtts://test.mosquitto.org:8884",
    // .client_cert_pem = (const char *)client_cert_pem_start,  // Chứng chỉ client
    // .client_key_pem = (const char *)client_key_pem_start,    // Khóa riêng của client
    // .skip_cert_common_name_check = true,                     // Bỏ qua kiểm tra hostname
    // .use_global_ca_store = true,  // Bỏ tùy chọn này
};
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

// void set_mqtt_data_handler(void*cb)
// {
//     mqtt_data_handler_pt=cb;
// }

