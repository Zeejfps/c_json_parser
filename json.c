#include <stdlib.h>
#include <string.h>
#include "json.h"

#define MAX_OBJECT_PROPERTY_COUNT 50
#define MAX_ARRAY_ELEMENTS_COUNT 50

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
    JsonElement* elements;
    size_t elements_count;
} JsonArray_t;

typedef struct JsonDoc_t {
    JsonElement root;
} JsonDoc_t;

JsonError parse_object(JsonParser_t* parser, FILE* file, JsonObject_t* object);
JsonError parse_array(JsonParser_t* parser, FILE* file, JsonArray_t* array);

JsonDoc_t* doc_create() {
    JsonDoc_t* doc = (JsonDoc_t*)calloc(1, sizeof(JsonDoc_t));
    return doc;
}

void doc_destroy(JsonDoc_t* doc) {
    if (doc == 0) {
        return;
    }

    element_destroy(doc->root);
    doc->root = 0;

    free(doc);
    doc = 0;
}

JsonError json_doc_create(JsonDoc* docHandle) {
    JsonDoc_t* doc = doc_create();
    *docHandle = doc;
    return JsonError_NONE;
}

void json_doc_destroy(JsonDoc* docHandle) {
    JsonDoc_t* doc = *docHandle;
    doc_destroy(doc);
    *docHandle = 0;
}

JsonElement_t* element_create() {
    return (JsonElement_t*)calloc(1, sizeof(JsonElement_t));
}

element_destroy(JsonElement_t* element) {
    if (element == 0) {
        return;
    }

    switch (element->kind)
    {
    
    case JsonElementKind_ARRAY:
        JsonArray_t* arr = (JsonArray_t*)element->value.array_value;
        array_destroy(arr);
        element->value.array_value = 0;
        break;
    
    case JsonElementKind_OBJECT:
        JsonObject_t* obj = (JsonObject_t*)element->value.obj_value;
        object_destroy(obj);
        element->value.obj_value = 0;
        break;

    case JsonElementKind_STRING:
        char* str = element->value.str_value;
        free(str);
        element->value.str_value = 0;
        break;

    default:
        break;
    }

    free(element);
}

JsonArray_t* array_create() {
    JsonArray_t* array = (JsonArray_t*)calloc(1, sizeof(JsonArray_t));
    array->elements = calloc(MAX_ARRAY_ELEMENTS_COUNT, sizeof(JsonElement_t*));
    return array;
}

void array_destroy(JsonArray_t* array){
    if (array == 0) {
        return;
    }

    for (int i = 0; i < array->elements_count; i++) {
        element_destroy(array->elements[i]);
        array->elements[i] = 0;
    }
    free(array->elements);
    array->elements = 0;

    free(array);
}

JsonObject_t* object_create() {
    JsonObject_t* obj = (JsonObject_t*)calloc(1, sizeof(JsonObject_t));
    obj->property_names = calloc(MAX_OBJECT_PROPERTY_COUNT, sizeof(char*));
    obj->property_values = calloc(MAX_OBJECT_PROPERTY_COUNT, sizeof(JsonElement_t*));
    return obj;
}

void object_destroy(JsonObject_t* obj) {
    if (obj == 0) {
        return;
    }

    for (int i = 0; i < obj->property_count; i++) {
        free(obj->property_names[i]);
        obj->property_names[i] = 0;

        element_destroy(obj->property_values[i]);
        obj->property_values[i] = 0;
    }
    free(obj->property_names);
    obj->property_names = 0;

    free(obj->property_values);
    obj-> property_values = 0;

    free(obj);
}

int read_token(FILE* file) {
    int c;
    while ((c = fgetc(file)) != EOF) {
        if (c != ' ' && c != '\r' && c != '\n'){
            return c;
        }
    }
    return c;
}

JsonError parser_begin_write(JsonParser_t* parser) {
    parser->write_head = 0;
    return JsonError_NONE;
}

JsonError parser_end_write(JsonParser_t* parser) {
    size_t index = parser->write_head;
    if (index >= 256) {
        return JsonError_PARSER_ERROR;
    }
    parser->buffer[index] = '\0';
    return JsonError_NONE;
}

JsonError parser_write_char(JsonParser_t* parser, int c) {
    if (parser->write_head >= 256) {
        return JsonError_PARSER_ERROR;
    }
    parser->buffer[parser->write_head++] = c;
    return JsonError_NONE;
}

JsonError parser_write_literal(JsonParser_t* parser, FILE* file) {
    int c;
    while ((c = fgetc(file)) != EOF) {
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))) {
            ungetc(c, file);
            return JsonError_NONE;
        }
        parser_write_char(parser, c);
    }
    return JsonError_PARSER_ERROR;
}

JsonError parse_string(JsonParser_t* parser, FILE* file, char** value) {
    parser->write_head = 0;
    int c;
    while ((c = fgetc(file)) != EOF) {
        if (c == '"') {
            size_t str_len = parser->write_head;
            char* str = malloc(sizeof(char) * (str_len + 1));
            memcpy(str, parser->buffer, sizeof(char) * str_len);
            *value = str;
            return JsonError_NONE;
        }
        JsonError err = parser_write_char(parser, c);
        if (err != JsonError_NONE) {
            return err;
        }
    }
    return JsonError_PARSER_ERROR;
}

JsonError read_name_value_separator(JsonParser_t* parser, FILE* file) {
    int c;
    while ((c = fgetc(file)) != EOF) {
        if (c == ':') {
            return JsonError_NONE;
        }
    }
    return JsonError_PARSER_ERROR;
}

JsonError parse_number(JsonParser_t* parser, FILE* file, float* value) {
    printf("parsing_number");
    int c;
    while ((c = fgetc(file)) != EOF) {
        if (c != '.' && (c < '0' || c > '9')) {
            ungetc(c, file);
            parser->buffer[parser->write_head] = '\0';
            char *end;
            *value = strtof(parser->buffer, &end);
            if (parser->buffer == end) {
                printf("Failed to parse float\n");
                return JsonError_PARSER_ERROR;
            }
            return JsonError_NONE;
        }
        JsonError err = parser_write_char(parser, c);
        if (err != JsonError_NONE) {
            printf("Failed to write char");
            return err;
        }
    }

    return JsonError_PARSER_ERROR;
}

JsonError parse_element(JsonParser_t* parser, FILE* file, JsonElement_t* element) {
    int c;
    while ((c = fgetc(file)) != EOF) {
        if (c == '"') {
            printf("parsing string\n");
            char* value;
            JsonError err = parse_string(parser, file, &value);
            if (err != JsonError_NONE){
                return err;
            }
            element->value.str_value = value;
            printf("Element Value: %s\n", value);
            return JsonError_NONE;
        }
        else if (c == 't' || c == 'f' || c == 'n') {
            parser_begin_write(parser);
            parser_write_char(parser, c);
            parser_write_literal(parser, file);
            parser_end_write(parser);

            if (strcmp(parser->buffer, "true") == 0) {
                element->kind = JsonElementKind_BOOLEAN;
                element->value.bool_value = 1;
                //printf("Element Value: true\n");
                return JsonError_NONE;
            }
            else if (strcmp(parser->buffer, "false") == 0) {
                element->kind = JsonElementKind_BOOLEAN;
                element->value.bool_value = 0;
                //printf("Element Value: false\n");
                return JsonError_NONE;
            }
            else if (strcmp(parser->buffer, "null") == 0) {
                element->kind = JsonElementKind_NULL;
                element->value.is_null = 1; // Do i need this?
                //printf("Element Value: null\n");
                return JsonError_NONE;
            }
            printf("Unknown litteral: %s\n", parser->buffer);
            return JsonError_PARSER_ERROR;
        }
        else if (c >= '0' && c <= '9') {
            parser_begin_write(parser);
            parser_write_char(parser, c);
            float value;
            JsonError err = parse_number(parser, file, &value);
            if (err != JsonError_NONE){
                printf("Failed to parse number\n");
                return err;
            }
            element->kind = JsonElementKind_NUMBER;
            element->value.float_value = value;
            printf("Property Value: %f\n", value);
            return JsonError_NONE;
        }
        else if (c == '[') {
            printf("we know we are here\n");
            JsonArray_t* arr = array_create();
            JsonError err = parse_array(parser, file, arr);
            if (err != JsonError_NONE) {
                printf("Failed to parse array\n");
                return err;
            }
            element->kind = JsonElementKind_ARRAY;
            element->value.array_value = arr;
            printf("Value is Array\n");
            return JsonError_NONE;
        }
        else if (c == '{') {
            JsonObject_t* obj = object_create();
            JsonError err = parse_object(parser, file, obj);
            if (err != JsonError_NONE) {
                printf("Failed to parse object\n");
                return err;
            }
            element->kind = JsonElementKind_OBJECT;
            element->value.obj_value = obj;
            return JsonError_NONE;
        }
    }
    return JsonError_PARSER_ERROR;
}

JsonError parse_object(JsonParser_t* parser, FILE* file, JsonObject_t* object) {
    printf("parsing object\n");
    int c;
    while ((c = read_token(file)) != EOF) {
        if (c == '"') {
            char* property_name;
            JsonError err = parse_string(parser, file, &property_name);
            if (err != JsonError_NONE){
                printf("Failed to parse string\n");
                return err;
            }
            printf("Property Name: %s\n", property_name);

            err = read_name_value_separator(parser, file);
            if (err != JsonError_NONE) {
                printf("Failed to read name-value separator");
                return err;
            }

            JsonElement_t* element = element_create();
            err = parse_element(parser, file, element);
            if (err != JsonError_NONE) {
                printf("Failed to parse property value\n");
                return err;
            }

            size_t property_index = object->property_count;
            object->property_count++;
            object->property_names[property_index] = property_name;
            object->property_values[property_index] = element;
        }
        else if (c == '}') {
            printf("Finished parsing object\n");
            return JsonError_NONE;
        }
        else if (c == ',') {

        }
        else {
            printf("Unexpected character encountered: %c\n", (char)c);
            return JsonError_PARSER_ERROR;
        }
    }
    return JsonError_PARSER_ERROR;
}

JsonError parse_array(JsonParser_t* parser, FILE* file, JsonArray_t* array) {
    printf("Parsing array...\n");
    int c;
    while ((c = read_token(file)) != EOF) {
        if (c == ']') {
            printf("Finished parsing array\n");
            return JsonError_NONE;
        }
        
        ungetc(c, file);

        JsonElement_t* element = element_create();
        JsonError err = parse_element(parser, file, element);
        if (err != JsonError_NONE) {
            return err;
        }
        array->elements[array->elements_count] = element;
        array->elements_count++;
    }
    return JsonError_PARSER_ERROR;
}

JsonError json_parse_file(
    JsonDoc docHandle, 
    FILE* file
) {
    JsonParser_t parser = {0};
    JsonDoc_t* doc = (JsonDoc_t*)docHandle; 
    JsonElement_t* root_element = element_create();

    int c;
    while ((c = read_token(file)) != EOF) {
        if (c == '{') {
            JsonObject_t* obj = object_create();
            JsonError err = parse_object(&parser, file, obj);
            if (err != JsonError_NONE) {
                object_destroy(obj);
                element_destroy(root_element);
                printf("Failed to parse obj\n");
                return err;
            }
            root_element->kind = JsonElementKind_OBJECT;
            root_element->value.obj_value = obj;
            break;
        }
        else if (c == '[') {
            JsonArray_t* array = array_create();
            JsonError err = parse_array(&parser, file, array);
            if (err != JsonError_NONE) {
                array_destroy(array);
                element_destroy(root_element);
                printf("Failed to parse array\n");
                return err;
            }
            root_element->kind = JsonElementKind_ARRAY;
            root_element->value.array_value = array;
            break;
        }
        else {
            printf("Json file must start with a root object or array");
            element_destroy(root_element);
            return JsonError_PARSER_ERROR;
        }
    }

    doc->root = root_element;
    return JsonError_NONE;
}

JsonError json_get_root_element(
    JsonDoc docHandle, 
    JsonElement* out_root
) {
    JsonDoc_t* doc = (JsonDoc_t*)docHandle;
    *out_root = doc->root;
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