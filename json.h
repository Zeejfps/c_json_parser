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
    JsonError_ARRAY_IS_NULL,
    JsonError_INDEX_OUT_OF_BOUNDS,
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

JsonError json_element_get_value_object(
    JsonElement element,
    JsonObject* out_object
);

JsonError json_element_get_value_string(
    JsonElement element,
    char** out_value
);

JsonError json_element_get_value_number(
    JsonElement element,
    float* out_value
);

JsonError json_element_get_value_array(
    JsonElement element,
    JsonArray* out_value
);

JsonError json_element_get_value_boolean(
    JsonElement element,
    char* out_value
);

JsonError json_object_get_property_by_name(
    JsonObject obj, 
    const char* name,
    JsonElement* out_property
);

JsonError json_array_get_length(
    JsonArray array,
    size_t* out_len
);

JsonError json_array_get_element_at_index(
    JsonArray array,
    size_t index,
    JsonElement* value
);

#endif