
#include "json_app.h"
#include "string.h"


// Hàm tạo chuỗi JSON
char* json_generator(int num_pairs, ...) {
    va_list args;
    va_start(args, num_pairs);
    
    static char json[1024]; // Giả sử kích thước tối đa là 1024 ký tự
    char* p = json;
    
    p += sprintf(p, "{");
    
    for (int i = 0; i < num_pairs; ++i) {
        if (i > 0) p += sprintf(p, ",");

        const char* key = va_arg(args, const char*);
        p += sprintf(p, "\"%s\":", key);

        const char* type = va_arg(args, const char*);
        if (strcmp(type, "string") == 0) {
            const char* value = va_arg(args, const char*);
            p += sprintf(p, "\"%s\"", value);
        } else if (strcmp(type, "int") == 0) {
            int value = va_arg(args, int);
            p += sprintf(p, "%d", value);
        } else if (strcmp(type, "double") == 0) {
            double value = va_arg(args, double);
            p += sprintf(p, "%f", value);
        } else if (strcmp(type, "bool") == 0) {
            int value = va_arg(args, int); // bool được promote thành int trong va_arg
            p += sprintf(p, value ? "true" : "false");
        }
    }
    
    p += sprintf(p, "}");
    va_end(args);
    
    return json;
}

// Hàm phân tích chuỗi JSON
struct KeyValue* json_parser(const char* json_str, int* num_pairs) {
    struct KeyValue* result = NULL;
    int count = 0;
    struct KeyValue kv;
    const char* p = json_str;
    
    // Bỏ qua khoảng trắng đầu tiên
    while (*p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) p++;

    // Kiểm tra xem chuỗi có bắt đầu bằng '{' không
    if (*p != '{') return NULL;
    p++;

    while (*p) {
        // Bỏ qua khoảng trắng
        while (*p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) p++;

        // Đọc key
        if (*p == '"') {
            p++; // Bỏ qua dấu ngoặc kép mở
            char* k = kv.key;
            while (*p && *p != '"' && (k - kv.key) < 63) {
                *k++ = *p++;
            }
            *k = '\0';
            if (*p == '"') p++; // Bỏ qua dấu ngoặc kép đóng
        }

        // Bỏ qua khoảng trắng và dấu hai chấm
        while (*p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == ':')) p++;

        // Đọc value
        if (*p == '"') {
            // Giá trị chuỗi
            p++; // Bỏ qua dấu ngoặc kép mở
            char* v = kv.value;
            while (*p && *p != '"' && (v - kv.value) < 63) {
                *v++ = *p++;
            }
            *v = '\0';
            if (*p == '"') p++; // Bỏ qua dấu ngoặc kép đóng
        } else {
            // Giá trị số hoặc boolean
            char* v = kv.value;
            while (*p && *p != ',' && *p != '}' && (v - kv.value) < 63) {
                *v++ = *p++;
            }
            *v = '\0';
        }

        // Thêm cặp key-value vào kết quả
        if (strlen(kv.key) > 0 && strlen(kv.value) > 0) {
            result = (struct KeyValue*) realloc(result, (count + 1) * sizeof(struct KeyValue));
            result[count++] = kv;
            memset(&kv, 0, sizeof(struct KeyValue));
        }

        // Bỏ qua khoảng trắng và dấu phẩy
        while (*p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == ',')) p++;

        // Kiểm tra kết thúc object
        if (*p == '}') break;
    }

    *num_pairs = count;
    return result;
}

int main() {
    // Tạo JSON
    char* json_str = json_generator(3, "name", "string", "John", "age", "int", 30, "married", "bool", 1);
    printf("Generated JSON: %s\n", json_str);

    // Phân tích JSON
    int num_pairs;
    struct KeyValue* parsed_pairs = json_parser(json_str, &num_pairs);

    printf("Parsed JSON:\n");
    for (int i = 0; i < num_pairs; ++i) {
        printf("Key: %s, Value: %s\n", parsed_pairs[i].key, parsed_pairs[i].value);
    }

    // Giải phóng bộ nhớ
    free(parsed_pairs);

    return 0;
}
