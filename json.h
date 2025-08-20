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
    JsonError_ELEMENT_KIND_MISSMATCH,
    JsonError_OBJECT_PROPERTY_NOT_FOUND,
} JsonError;

JsonError json_doc_create(JsonDoc* doc);
void json_doc_destroy(JsonDoc* doc);

JsonError json_parse_file(
    JsonDoc doc, 
    FILE* file
);

JsonError json_doc_get_root_element(
    JsonDoc doc, 
    JsonElement* out_root
);

JsonError json_element_get_object_value(
    JsonElement element,
    JsonObject* out_object
);

JsonError json_element_get_value_str(
    JsonElement element,
    char** out_value
);

JsonError json_element_get_value_str_len(
    JsonDoc doc,
    JsonElement element, 
    u_int32_t* len
);

JsonError json_object_get_property_by_name(
    JsonObject obj, 
    const char* name,
    JsonElement* out_property
);

#endif