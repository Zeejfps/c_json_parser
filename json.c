#include <stdlib.h>
#include "json.h"

#define MAX_PARSERS 1

typedef struct JsonParser_t {
    enum JsonParserState {
        JsonParserState_READ_ROOT_ELEMENT,

        JsonParserState_BEGIN_READ_ARRAY,
        JsonParserState_END_READ_ARRAY,

        JsonParserState_BEGIN_READ_OBJECT,
        JsonParserState_END_READ_OBJECT,

        JsonParserState_READ_PROPERTY_NAME,
        JsonParserState_READ_PROPERTY_VALUE,

        JsonParserState_READ_ELEMENT,

        JsonParserState_READ_STRING,
        JsonParserState_END_READ_STRING,

        JsonParserState_READ_ELEMENT_LITERAL,
        JsonParserState_READ_ELEMENT_NUMBER,
        JsonParserState_READ_NEXT_ELEMENT,

        JsonParserState_READING_ARRAY_VALUE_STRING,
        JsonParserState_READING_ARRAY_VALUE_LITERAL,
        JsonParserState_READING_ARRAY_VALUE_NUMBER,
        JsonParserState_READING_ARRAY_VALUE_ARRAY,
        JsonParserState_READING_ARRAY_VALUE_OBJECT,
        JsonParserState_READING_ARRAY_VALUE_SEPARATOR,
    } state;
    enum ElementKind {
        ElementKind_OBJECT,
        ElementKind_OBJECT_PROPERTY_NAME,
        ElementKind_OBJECT_PROPERTY_VALUE,
        ElementKind_ARRAY,
        ElementKind_ARRAY_VALUE,
    } element_kind;
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
        .state = JsonParserState_READ_ROOT_ELEMENT
    };
    JsonDoc_t* doc = (JsonDoc_t*)docHandle; 

    int c;
    while ((c = fgetc(file)) != EOF) {
        switch (parser.state)
        {

        case JsonParserState_READ_ROOT_ELEMENT:
            if (c == '{') {
                parser.state = JsonParserState_READ_STRING;
                parser.element_kind = ElementKind_OBJECT_PROPERTY_NAME;
            }
            else if (c == '[') {
                parser.state = JsonParserState_BEGIN_READ_ARRAY;
            }
            else if (c != ' ' && c != '\r' && c != '\n') {
                printf("Json file must start with a root object or array");
                return JsonError_PARSER_ERROR;
            }
            break;

        case JsonParserState_BEGIN_READ_OBJECT:
            if (c == '"'){
                parser.state = JsonParserState_READ_PROPERTY_NAME;
            }
            else if (c == '}') {
                parser.state = JsonParserState_END_READ_OBJECT;
            }
            else if (c != ' ' && c != '\r' && c != '\n') {
                printf("Invalid character: %c\n", (char)c);
                return JsonError_PARSER_ERROR;
            }
            break;
        
        case JsonParserState_END_READ_OBJECT:
            if (c == ',') {
                parser.state = JsonParserState_READ_NEXT_ELEMENT;
            }
            break;

        case JsonParserState_READ_NEXT_ELEMENT:
            if (c == '{') {
                parser.state = JsonParserState_BEGIN_READ_OBJECT;
            }
            else if (c == '"') {
                parser.state = JsonParserState_READ_PROPERTY_NAME;
            }
            else if (c != ' ' && c != '\r' && c != '\n') {
                return JsonError_PARSER_ERROR;
            }
            break;

        case JsonParserState_READ_ELEMENT:
            if (c == '"') {
                parser.state = JsonParserState_READ_STRING;
            }
            else if (c == 't' || c == 'f' || c == 'n') {
                parser.state = JsonParserState_READ_ELEMENT_LITERAL;
            }
            else if (c >= '0' && c <= '9') {
                parser.state = JsonParserState_READ_ELEMENT_NUMBER;
            }
            else if (c == '[') {
                parser.state = JsonParserState_BEGIN_READ_ARRAY;
            }
            else if (c == '{') {
                parser.state = JsonParserState_BEGIN_READ_OBJECT;
            }
            break;

        case JsonParserState_READ_STRING:
            if (c == '"') {
                parser.state = JsonParserState_END_READ_STRING;
                if (parser.element_kind == ElementKind_OBJECT_PROPERTY_NAME) {
                    // Save property name
                }
                else if (parser.element_kind == ElementKind_OBJECT_PROPERTY_VALUE) {
                    // Save property value
                }
                else if (parser.element_kind == ElementKind_ARRAY_VALUE) {
                    // Save array value
                }
            }
            break;
        
        case JsonParserState_END_READ_STRING:
            if (c == ':' && parser.element_kind == ElementKind_OBJECT_PROPERTY_NAME) {
                parser.state = JsonParserState_READ_PROPERTY_NAME;
            }
            break;

        case JsonParserState_READ_ELEMENT_LITERAL:
            if (c == ' '){
                //TODO: check if its true, false, or null
                //parser.state = JsonParserState_READING_PROPERTY_SEPARATOR;
            }
            else if (c == ',') {
                parser.state = JsonParserState_BEGIN_READ_OBJECT;
            }
            else if (c == '}') {
                
            }
            else {

            }
            break;
        
        case JsonParserState_READ_ELEMENT_NUMBER:
            if (c == ' '){
                //TODO: check if its true, false, or null
                //parser.state = JsonParserState_READING_PROPERTY_SEPARATOR;
            }
            else if (c == ',') {
                parser.state = JsonParserState_BEGIN_READ_OBJECT;
            }
            else if (c == '}') {
                
            }            
            break;

        default:
            break;
        }
    }

    printf("State %d\n", parser.state);
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