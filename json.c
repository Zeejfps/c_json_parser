#include <stdlib.h>
#include "json.h"

#define MAX_PARSERS 1

typedef struct JsonParser_t {
    u_int32_t test;
} JsonParser_t;


JsonError json_parser_create(JsonParser* parserHandle) {
    JsonParser_t* parser = (JsonParser_t*)calloc(1, sizeof(JsonParser_t));
    parserHandle = parser;
    return NONE;
}

void json_parser_destroy(JsonParser* parserHandle) {
    JsonParser_t* parser = (JsonParser_t*)(*parserHandle);
    free(parser);
    parserHandle = 0;
}

JsonError json_doc_create(JsonDoc* docHandle) {
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
    JsonParser parserHandle,
    JsonDoc docHandle, 
    FILE* file
) {
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