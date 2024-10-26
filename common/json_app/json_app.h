#ifndef JSON_APP_H
#define JSON_APP_H
    
#include <stdarg.h>
#include "stdio.h"
#include "string.h"
// Cấu trúc để lưu trữ cặp key-value
struct KeyValue {
    char key[64];
    char value[64];
};

char* json_generator(int num_pairs, ...);
struct KeyValue* json_parser(const char* json_str, int* num_pairs);
#endif