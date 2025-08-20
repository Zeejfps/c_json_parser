#ifndef JSON_H
#define JSON_H

#include <stdio.h>
#include <stdint.h>

typedef void* JsonDoc;
typedef void* JsonElement;
typedef void* JsonObject;
typedef void* JsonArray;

typedef enum JsonError { 
    JsonError_NONE, 
    JsonError_UNKNOWN_ERROR,
    JsonError_PARSER_ERROR,
} JsonError;

JsonError json_doc_create(JsonDoc* doc);
void json_doc_destroy(JsonDoc* doc);

JsonError json_parse_file(
    JsonDoc doc, 
    FILE* file
);

JsonError json_get_root_element(
    JsonDoc doc, 
    JsonElement* out_root
);

JsonError json_get_element_child_by_name(
    JsonDoc doc, 
    JsonElement parent, 
    const char* name,
    JsonElement* out_child
);

JsonError json_get_element_value_str(
    JsonDoc doc,
    JsonElement element,
    const char* out_value
);

JsonError json_get_element_value_str_len(
    JsonDoc doc,
    JsonElement element, 
    u_int32_t* len
);

#endif