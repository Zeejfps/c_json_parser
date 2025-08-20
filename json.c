#include <stdlib.h>
#include "json.h"

#define MAX_PARSERS 1

typedef struct JsonParser_t {
    enum JsonParserState {
        JsonParserState_READING_ROOT_ELEMENT,
        JsonParserState_READING_OBJECT_ELEMENT,
        JsonParserState_READING_ARRAY_ELEMENT,
        JsonParserState_READING_PROPERTY_NAME,
        JsonParserState_READING_KEY_VALUE_SEPARATOR,
        JsonParserState_READING_PROPERTY_VALUE,
        JsonParserState_READING_PROPERTY_VALUE_STRING,
        JsonParserState_READING_PROPERTY_SEPARATOR,
    } state;
    u_int32_t test;
} JsonParser_t;

typedef struct JsonDoc_t {
    u_int32_t test;
} JsonDoc_t;

JsonError json_doc_create(JsonDoc* docHandle) {
    JsonDoc_t* doc = (JsonDoc_t*)calloc(1, sizeof(JsonDoc_t));
    *docHandle = doc;
    return JsonError_NONE;
}

void json_doc_destroy(JsonDoc* docHandle) {
    docHandle = 0;
}

JsonError json_element_create(JsonElement* elementHandle, JsonDoc doc) {
    return JsonError_NONE;
}

void json_element_destroy(JsonElement* elementHandle) {
    elementHandle = 0;
}

JsonError json_parse_file(
    JsonDoc docHandle, 
    FILE* file
) {
    //TODO: Error Check

    JsonParser_t parser = {
        .state = JsonParserState_READING_ROOT_ELEMENT
    };
    JsonDoc_t* doc = (JsonDoc_t*)docHandle; 

    int c;
    while ((c = fgetc(file)) != EOF) {
        switch (parser.state)
        {

        case JsonParserState_READING_ROOT_ELEMENT:
            if (c == '{') {
                parser.state = JsonParserState_READING_OBJECT_ELEMENT;
            }
            else if (c == '[') {
                parser.state = JsonParserState_READING_ARRAY_ELEMENT;
            }
            else {
                return JsonError_PARSER_ERROR;
            }
            break;

        case JsonParserState_READING_OBJECT_ELEMENT:
            if (c == '"'){
                parser.state = JsonParserState_READING_PROPERTY_NAME;
            }
            else {
                return JsonError_PARSER_ERROR;
            }
            break;

        case JsonParserState_READING_PROPERTY_NAME:
            if (c == '"') {
                parser.state = JsonParserState_READING_KEY_VALUE_SEPARATOR;
            }
            break;
        
        case JsonParserState_READING_KEY_VALUE_SEPARATOR:
            if (c == ':') {
                parser.state = JsonParserState_READING_PROPERTY_VALUE;
            }
            break;

        case JsonParserState_READING_PROPERTY_VALUE:
            if (c == '"') {
                parser.state = JsonParserState_READING_PROPERTY_VALUE_STRING;
            }
            break;

        case JsonParserState_READING_PROPERTY_VALUE_STRING:
            if (c == '"') {
                parser.state = JsonParserState_READING_PROPERTY_SEPARATOR;
            }
            break;

        case JsonParserState_READING_PROPERTY_SEPARATOR:
            if (c == ',') {
                parser.state = JsonParserState_READING_OBJECT_ELEMENT;
            }
            else if (c == '}') {
                
            }
            break;

        default:
            break;
        }
    }

    return JsonError_NONE;
}

JsonError json_get_root_element(
    JsonDoc docHandle, 
    JsonElement* out_root
) {
    return JsonError_NONE;
}

JsonError json_get_element_child_by_name(
    JsonDoc doc, 
    JsonElement parent, 
    const char* name,
    JsonElement* out_child
) {
    return JsonError_NONE;
}

JsonError json_get_element_value_str(
    JsonDoc doc,
    JsonElement element,
    const char* out_value
){
    return JsonError_NONE;
}

JsonError json_get_element_value_str_len(
    JsonDoc doc,
    JsonElement element, 
    u_int32_t* len
) {
    return JsonError_NONE;
}