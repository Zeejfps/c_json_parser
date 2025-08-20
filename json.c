#include <stdlib.h>
#include <string.h>
#include "json.h"

typedef struct JsonParser_t {
    char buffer[256];
    size_t write_head;
} JsonParser_t;

typedef struct JsonElement_t {
    enum JsonElementKind {
        JsonElementKind_NULL,
        JsonElementKind_NUMBER,
        JsonElementKind_BOOLEAN,
        JsonElementKind_STRING,
        JsonElementKind_OBJECT,
        JsonElementKind_ARRAY,
    } kind;
    union value
    {
        char is_null;
        float float_value;
        char bool_value;
        char* str_value;
        JsonObject obj_value;
        JsonArray array_value;
    } value;
} JsonElement_t;

typedef struct JsonObject_t {
    char** property_names;
    JsonElement_t** property_values;
    size_t property_count;
} JsonObject_t;

typedef struct JsonArray_t {
    JsonElement* values;
    size_t value_count;
} JsonArray_t;

typedef struct JsonDoc_t {
    JsonElement root;
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

JsonElement_t* create_element() {
    return (JsonElement_t*)calloc(1, sizeof(JsonElement_t));
}

JsonArray_t* create_array() {
    return (JsonArray_t*)calloc(1, sizeof(JsonArray_t));
}

JsonObject_t* create_object() {
    return (JsonObject_t*)calloc(1, sizeof(JsonObject_t));
}

JsonError parse_string(JsonParser_t* parser, FILE* file) {
    int c;
    while ((c = fgetc(file)) != EOF) {
        if (c == '"') {
            return JsonError_NONE;
        }
        if (parser->write_head >= 256) {
            return JsonError_PARSER_ERROR;
        }
        parser->buffer[parser->write_head++] = c;
    }
}

JsonError parse_object(JsonParser_t* parser, FILE* file, JsonObject_t* object) {
    int c;
    while ((c = fgetc(file)) != EOF) {
        if (c == '"') {
            JsonError err = parse_string(parser, file);
            if (err != JsonError_NONE)
                return err;
            size_t property_name_len = parser->write_head;
            char* property_name = malloc(sizeof(char) * property_name_len + 1);
            memcpy(property_name, parser->buffer, sizeof(char) * property_name_len);

            JsonElement_t* element = create_element();
            parse_element(parser, file, element);

            size_t property_index = object->property_count;
            object->property_count++;
            object->property_names[property_index] = property_name;
            object->property_values[property_index] = element;
        }
        else if (c == '}') {
            return JsonError_NONE;
        }
        else if (c != ' ' && c != '\r' && c != '\n') {
            return JsonError_PARSER_ERROR;
        }
    }
}

JsonError parse_array(JsonParser_t* parser, FILE* file, JsonArray_t* array) {

}

JsonError json_parse_file(
    JsonDoc docHandle, 
    FILE* file
) {
    //TODO: Error Check

    JsonParser_t parser = {0};
    JsonDoc_t* doc = (JsonDoc_t*)docHandle; 

    JsonElement_t* root_element = create_element();

    int c;
    while ((c = fgetc(file)) != EOF) {

        if (c == '{') {
            JsonObject_t* obj = create_object();
            JsonError err = parse_object(&parser, file, obj);
            if (err != JsonError_NONE) {
                return err;
            }
            root_element->value.obj_value = obj;
            break;
        }
        else if (c == '[') {
            JsonArray_t* arr = create_array();
            JsonError err = parse_array(&parser, file, arr);
            if (err != JsonError_NONE) {
                return err;
            }
            root_element->value.array_value = arr;
            break;
        }
        else if (c != ' ' && c != '\r' && c != '\n') {
            printf("Json file must start with a root object or array");
            return JsonError_PARSER_ERROR;
        }
        // case JsonParserState_READ_ELEMENT:
        //     if (c == '"') {
        //         parser.state = JsonParserState_READ_STRING;
        //     }
        //     else if (c == 't' || c == 'f' || c == 'n') {
        //         parser.state = JsonParserState_READ_ELEMENT_LITERAL;
        //     }
        //     else if (c >= '0' && c <= '9') {
        //         parser.state = JsonParserState_READ_ELEMENT_NUMBER;
        //     }
        //     else if (c == '[') {
        //         parser.state = JsonParserState_READ_ARRAY;
        //     }
        //     else if (c == '{') {
        //         parser.state = JsonParserState_READ_OBJECT;
        //     }
        //     break;
    }

    doc->root = root_element;

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