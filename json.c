#include <stdlib.h>
#include "json.h"

#define MAX_PARSERS 1

typedef struct JsonParser_t {
    u_int32_t test;
} JsonParser_t;

typedef struct JsonDoc_t {
    u_int32_t test;
} JsonDoc_t;

JsonError json_doc_create(JsonDoc* docHandle) {
    JsonDoc_t* doc = (JsonDoc_t*)calloc(1, sizeof(JsonDoc_t));
    *docHandle = doc;
    return NONE;
}

void json_doc_destroy(JsonDoc* docHandle) {
    docHandle = 0;
}

JsonError json_element_create(JsonElement* elementHandle, JsonDoc doc) {
    return NONE;
}

void json_element_destroy(JsonElement* elementHandle) {
    elementHandle = 0;
}

JsonError json_parse_file(
    JsonDoc docHandle, 
    FILE* file
) {
    //TODO: Error Check

    JsonDoc_t* doc = (JsonDoc_t*)docHandle; 

    int c;
    while ((c = fgetc(file)) != EOF) {

    }

    return NONE;
}

JsonError json_get_root_element(
    JsonDoc docHandle, 
    JsonElement* out_root
) {
    return NONE;
}

JsonError json_get_element_child_by_name(
    JsonDoc doc, 
    JsonElement parent, 
    const char* name,
    JsonElement* out_child
) {
    return NONE;
}

JsonError json_get_element_value_str(
    JsonDoc doc,
    JsonElement element,
    const char* out_value
){
    return NONE;
}

JsonError json_get_element_value_str_len(
    JsonDoc doc,
    JsonElement element, 
    u_int32_t* len
) {
    return NONE;
}