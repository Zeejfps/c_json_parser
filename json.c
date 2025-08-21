#include <stdlib.h>
#include <string.h>
#include "json.h"

#define MAX_OBJECT_PROPERTY_COUNT 50
#define MAX_ARRAY_ELEMENTS_COUNT 50
#define INITIAL_PARSER_BUFFER_SIZE 512

#define Byte char
#define Bool char

// Forward Declarations
typedef struct JsonObject_t JsonObject_t;
typedef struct JsonArray_t JsonArray_t;
typedef struct JsonElement_t JsonElement_t;
typedef struct JsonString_t JsonString_t;

static JsonString_t* string_create();
static void string_destroy(JsonString_t* string);

static JsonElement_t* element_create();
static void element_destroy(JsonElement_t* element);

static JsonArray_t* array_create();
static void array_destroy(JsonArray_t* array);

static JsonObject_t* object_create();
static void object_destroy(JsonObject_t* obj);

typedef struct JsonParser_t {
    Byte buffer[INITIAL_PARSER_BUFFER_SIZE];
    size_t buffer_size;
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
        Bool bool_value;
        JsonString_t* str_value;
        JsonObject_t* obj_value;
        JsonArray_t* array_value;
    } value;
} JsonElement_t;

typedef struct JsonObject_t {
    JsonString_t** property_names;
    JsonElement_t** property_values;
    size_t property_names_array_size;
    size_t property_count;
} JsonObject_t;

typedef struct JsonArray_t {
    JsonElement_t** elements;
    size_t elements_array_size;
    size_t elements_count;
} JsonArray_t;

typedef struct JsonDoc_t {
    JsonElement root;
} JsonDoc_t;

typedef struct JsonString_t {
    Byte* bytes;
    size_t bytes_count;
    size_t length;
} JsonString_t;

static JsonString_t* string_create() {
    JsonString_t* string = (JsonString_t*)calloc(1, sizeof(JsonString_t));
    return string;
}

static void string_destroy(JsonString_t* string) {

    if (string == 0){
        return;
    }

    if (string->bytes != 0) {
        free(string->bytes);
        string->bytes = 0;
    }

    string->bytes_count = 0;
    string->length = 0;

    free(string);
    string = 0;
}

static JsonDoc_t* doc_create() {
    JsonDoc_t* doc = (JsonDoc_t*)calloc(1, sizeof(JsonDoc_t));
    return doc;
}

static void doc_destroy(JsonDoc_t* doc) {
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

static JsonElement_t* element_create() {
    return (JsonElement_t*)calloc(1, sizeof(JsonElement_t));
}

static void element_destroy(JsonElement_t* element) {
    if (element == 0) {
        return;
    }

    if (element->kind == JsonElementKind_ARRAY) {
        JsonArray_t* arr = element->value.array_value;
        array_destroy(arr);
        element->value.array_value = 0;
    }
    else if (element->kind == JsonElementKind_OBJECT) {
        JsonObject_t* obj = element->value.obj_value;
        object_destroy(obj);
        element->value.obj_value = 0;
    }
    else if (element->kind == JsonElementKind_STRING) {
        JsonString_t* str = element->value.str_value;
        string_destroy(str);
        element->value.str_value = 0;
    }

    free(element);
    element = 0;
}

static JsonArray_t* array_create() {
    JsonArray_t* array = (JsonArray_t*)calloc(1, sizeof(JsonArray_t));
    array->elements = calloc(MAX_ARRAY_ELEMENTS_COUNT, sizeof(JsonElement_t*));
    array->elements_array_size = MAX_ARRAY_ELEMENTS_COUNT;
    return array;
}

static void array_destroy(JsonArray_t* array) {
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
    array = 0;
}

static JsonObject_t* object_create() {
    JsonObject_t* obj = (JsonObject_t*)calloc(1, sizeof(JsonObject_t));
    obj->property_names = calloc(MAX_OBJECT_PROPERTY_COUNT, sizeof(JsonString_t*));
    obj->property_names_array_size = MAX_OBJECT_PROPERTY_COUNT;
    obj->property_values = calloc(MAX_OBJECT_PROPERTY_COUNT, sizeof(JsonElement_t*));
    return obj;
}

static void object_destroy(JsonObject_t* obj) {
    if (obj == 0) {
        return;
    }

    for (int i = 0; i < obj->property_count; i++) {
        string_destroy(obj->property_names[i]);
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

static int32_t read_utf8_code_point(FILE *f) {
    int c1 = fgetc(f);
    if (c1 == EOF) {
        return -1;
    }

    unsigned char byte1 = (unsigned char)c1;

    // 1-byte ASCII (0xxxxxxx)
    if ((byte1 & 0x80) == 0) {
        return byte1;
    }

    // Determine the number of bytes in the sequence
    int bytes_needed;
    int32_t codepoint;

    if ((byte1 & 0xE0) == 0xC0) {        // 2-byte sequence (110xxxxx)
        bytes_needed = 1;
        codepoint = byte1 & 0x1F;
    } else if ((byte1 & 0xF0) == 0xE0) { // 3-byte sequence (1110xxxx)
        bytes_needed = 2;
        codepoint = byte1 & 0x0F;
    } else if ((byte1 & 0xF8) == 0xF0) { // 4-byte sequence (11110xxx)
        bytes_needed = 3;
        codepoint = byte1 & 0x07;
    } else {
        // Invalid start byte
        return -1;
    }

    // Read and validate continuation bytes
    for (int i = 0; i < bytes_needed; ++i) {
        int next_byte = fgetc(f);
        if (next_byte == EOF) {
            return -1; // Unexpected EOF
        }
        unsigned char b = (unsigned char)next_byte;
        if ((b & 0xC0) != 0x80) {
            // Not a continuation byte
            return -1;
        }
        codepoint = (codepoint << 6) | (b & 0x3F);
    }

    // Check for overlong encodings and other invalid code points
    switch (bytes_needed) {
        case 1:
            if (codepoint < 0x80) return -1; // Overlong 2-byte
            break;
        case 2:
            if (codepoint < 0x800) return -1; // Overlong 3-byte
            break;
        case 3:
            if (codepoint < 0x10000) return -1; // Overlong 4-byte
            break;
    }

    if (codepoint > 0x10FFFF || (codepoint >= 0xD800 && codepoint <= 0xDFFF)) {
        return -1; // Invalid code point
    }

    return codepoint;
}

static int read_next_token(FILE* file) {
    int c;
    while ((c = read_utf8_code_point(file)) != EOF) {
        if (c != ' ' && c != '\r' && c != '\n'){
            return c;
        }
    }
    return c;
}

static JsonError parser_begin_write(JsonParser_t* parser) {
    parser->write_head = 0;
    return JsonError_NONE;
}

static JsonError parser_end_write(JsonParser_t* parser) {
    size_t index = parser->write_head;
    if (index >= parser->buffer_size) {
        return JsonError_PARSER_ERROR;
    }
    parser->buffer[index] = '\0';
    return JsonError_NONE;
}

static JsonError parser_write_char(JsonParser_t* parser, int c) {
    if (parser->write_head >= parser->buffer_size) {
        return JsonError_PARSER_ERROR;
    }
    parser->buffer[parser->write_head++] = c;
    return JsonError_NONE;
}

static JsonError parser_write_literal(JsonParser_t* parser, FILE* file) {
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

JsonError read_name_value_separator(JsonParser_t* parser, FILE* file) {
    int c;
    while ((c = fgetc(file)) != EOF) {
        if (c == ':') {
            return JsonError_NONE;
        }
    }
    return JsonError_PARSER_ERROR;
}

JsonError parse_string(JsonParser_t* parser, FILE* file, JsonString_t* value) {
    parser->write_head = 0;
    int c;
    while ((c = fgetc(file)) != EOF) {
        if (c == '"') {
            size_t str_len = parser->write_head;
            size_t bytes_count = str_len + 1;
            char* bytes = malloc(bytes_count);
            memcpy(bytes, parser->buffer, sizeof(char) * str_len);
            value->bytes = bytes;
            value->bytes_count = bytes_count;
            value->length = str_len;
            return JsonError_NONE;
        }
        JsonError err = parser_write_char(parser, c);
        if (err != JsonError_NONE) {
            printf("Failed to write char: 0x%04X\n", c);
            return err;
        }
    }
    printf("Closing brace not found\n");
    return JsonError_PARSER_ERROR;
}

JsonError parse_number(JsonParser_t* parser, FILE* file, float* value) {
    //printf("parsing_number");
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
        
JsonError parse_property_name(JsonParser_t* parser, FILE* file, JsonString_t** out_name) {

    JsonString_t* name_str = string_create();
    JsonError err = parse_string(parser, file, name_str);
    if (err != JsonError_NONE) {
        string_destroy(name_str);
        printf("Failed to parse string\n");
        return err;
    }

    *out_name = name_str;
    return JsonError_NONE;
}

JsonError parse_value(JsonParser_t* parser, FILE* file, JsonElement_t** out_element) {
    uint32_t token = read_next_token(file);
    if (token == EOF) {
        return JsonError_PARSER_ERROR;
    }

    if (token == '"') {
        JsonString_t* value = string_create();
        JsonError err = parse_string(parser, file, value);
        if (err != JsonError_NONE){
            printf("Failed to parse string\n");
            return err;
        }
        JsonElement_t* element = element_create();
        element->kind = JsonElementKind_STRING;
        element->value.str_value = value;
        *out_element = element;
        return JsonError_NONE;
    }
    else if (token == 't' || token == 'f' || token == 'n') {
        parser_begin_write(parser);
        parser_write_char(parser, token);
        parser_write_literal(parser, file);
        parser_end_write(parser);

        JsonElement_t* element = element_create();
        if (strcmp(parser->buffer, "true") == 0) {
            element->kind = JsonElementKind_BOOLEAN;
            element->value.bool_value = 1;
            *out_element = element;
            return JsonError_NONE;
        }
        else if (strcmp(parser->buffer, "false") == 0) {
            element->kind = JsonElementKind_BOOLEAN;
            element->value.bool_value = 0;
            *out_element = element;
            return JsonError_NONE;
        }
        else if (strcmp(parser->buffer, "null") == 0) {
            element->kind = JsonElementKind_NULL;
            element->value.is_null = 1; // Do i need this?
            *out_element = element;
            return JsonError_NONE;
        }
        printf("Unknown litteral: %s\n", parser->buffer);
        return JsonError_PARSER_ERROR;
    }
    else if (token >= '0' && token <= '9') {
        parser_begin_write(parser);
        parser_write_char(parser, token);
        float value;
        JsonError err = parse_number(parser, file, &value);
        if (err != JsonError_NONE){
            printf("Failed to parse number\n");
            return err;
        }
        JsonElement_t* element = element_create();
        element->kind = JsonElementKind_NUMBER;
        element->value.float_value = value;
        *out_element = element;
        //printf("Element Num Value: %f\n", value);
        return JsonError_NONE;
    }
    else if (token == '[') {
        JsonArray_t* arr = array_create();
        JsonElement_t* element = element_create();
        element->kind = JsonElementKind_ARRAY;
        element->value.array_value = arr;
        *out_element = element;
        return JsonError_NONE;
    }
    else if (token == '{') {
        JsonObject_t* obj = object_create();
        JsonElement_t* element = element_create();
        element->kind = JsonElementKind_OBJECT;
        element->value.obj_value = obj;
        *out_element = element;
        return JsonError_NONE;
    }

    printf("Unexpected token '%c'\n", token);
    return JsonError_PARSER_ERROR;
}

typedef struct StackFrame {
    enum {
        PARSE_OBJECT,
        PARSE_ARRAY
    } kind;
    void* element;
} StackFrame;

JsonError json_parse_file(
    JsonDoc docHandle, 
    FILE* file
) {
    JsonParser_t parser = {
        .buffer_size = INITIAL_PARSER_BUFFER_SIZE,
    };
    JsonDoc_t* doc = (JsonDoc_t*)docHandle; 
    JsonElement_t* root_element = element_create();

    uint32_t token = read_next_token(file);
    //printf("Token: %c\n", token);
    if (token == EOF) {
        element_destroy(root_element);
        return JsonError_PARSER_ERROR;
    }

    StackFrame stack[10];
    int32_t top = -1;

    if (token == '{') {
        JsonObject_t* obj = object_create();
        root_element->kind = JsonElementKind_OBJECT;
        root_element->value.obj_value = obj;
        top++;
        stack[top].kind = PARSE_OBJECT;
        stack[top].element = obj;
        //printf("pushing obj\n");
    }
    else if (token == '[') {
        JsonArray_t* array = array_create();
        root_element->kind = JsonElementKind_ARRAY;
        root_element->value.array_value = array;
        top++;
        stack[top].kind = PARSE_ARRAY;
        stack[top].element = array;
        //printf("pushing array\n");
    }
    else {
        printf("Json file must start with root object or array");
        return JsonError_PARSER_ERROR;
    }

    JsonError err;
    //printf("Top: %d\n", top);
    while (top > -1) {
        //printf("Processing frame\n}");
        StackFrame frame = stack[top];
        if (frame.kind == PARSE_OBJECT) {
            //printf("parsing object\n");
            JsonObject_t* curr_obj = (JsonObject_t*)frame.element;

            int32_t token = read_next_token(file);
            if (token == '"') {
                
                JsonString_t* property_name;
                err = parse_property_name(&parser, file, &property_name);
                if (err != JsonError_NONE) {
                    printf("Failed to parse property name\n");
                    return err;
                }

                //printf("Property name: %s\n", property_name->bytes);

                err = read_name_value_separator(&parser, file);
                if (err != JsonError_NONE) {
                    printf("Failed to read name value separator\n");
                    return err;
                }

                JsonElement_t* property_value;
                err = parse_value(&parser, file, &property_value);
                if (err != JsonError_NONE) {
                    printf("Failed to parse property value\n");
                    return err;
                }

                size_t propertyIndex = curr_obj->property_count;
                if (propertyIndex >= curr_obj->property_names_array_size) {
                    return JsonError_INDEX_OUT_OF_BOUNDS;
                }
                curr_obj->property_count++;
                curr_obj->property_names[propertyIndex] = property_name;
                curr_obj->property_values[propertyIndex] = property_value;

                if (property_value->kind == JsonElementKind_OBJECT) {
                    top++;
                    stack[top].element = property_value->value.obj_value;
                    stack[top].kind = PARSE_OBJECT;
                    continue;
                }

                if (property_value->kind == JsonElementKind_ARRAY) {
                    top++;
                    stack[top].element = property_value->value.array_value;
                    stack[top].kind = PARSE_ARRAY;
                    continue;
                }

                token = read_next_token(file);
            }

            if (token == ',') {
                continue;
            }

            if (token != '}') {
                printf("Expected '}' token, found %c\n", token);
                return JsonError_PARSER_ERROR;
            }

            size_t prev_frame_index = top - 1;
            if (prev_frame_index > -1) {
                StackFrame prev_frame = stack[prev_frame_index];
                if (prev_frame.kind == PARSE_OBJECT) {
                    JsonObject_t* prev_obj = (JsonObject_t*)prev_frame.element;
                    prev_obj->property_values[prev_obj->property_count-1]->value.obj_value = curr_obj;
                }
                else if (prev_frame.kind == PARSE_ARRAY) {
                    JsonArray_t* prev_arr = (JsonArray_t*)prev_frame.element;
                    prev_arr->elements[prev_arr->elements_count-1]->value.obj_value = curr_obj;
                }
            }
            top -= 1;
            //printf("finished parsing object\n");

        } else if (frame.kind == PARSE_ARRAY) {
            //printf("parsing array\n");
            JsonArray_t* curr_arr = (JsonArray_t*)frame.element;    

            int32_t token = read_next_token(file);
            if (token == ',') {
                continue;
            }

            if (token == ']') {
                size_t prev_frame_index = top - 1;
                if (prev_frame_index > -1) {
                    StackFrame prev_frame = stack[prev_frame_index];
                    if (prev_frame.kind == PARSE_OBJECT) {
                        JsonObject_t* prev_obj = (JsonObject_t*)prev_frame.element;
                        prev_obj->property_values[prev_obj->property_count-1]->value.array_value = curr_arr;
                    }
                    else if (prev_frame.kind == PARSE_ARRAY) {
                        JsonArray_t* prev_arr = (JsonArray_t*)prev_frame.element;
                        prev_arr->elements[prev_arr->elements_count-1]->value.array_value = curr_arr;
                    }
                }
                top -= 1;
                continue;
            }

            ungetc(token, file);

            JsonElement_t* array_value;
            err = parse_value(&parser, file, &array_value);
            if (err != JsonError_NONE) {
                printf("Failed to parse array value\n");
                return err;
            }

            size_t element_index = curr_arr->elements_count;
            if (element_index >= curr_arr->elements_array_size) {
                return JsonError_INDEX_OUT_OF_BOUNDS;
            }
            curr_arr->elements_count++;
            curr_arr->elements[element_index] = array_value;

            if (array_value->kind == JsonElementKind_OBJECT) {
                top++;
                stack[top].element = array_value->value.obj_value;
                stack[top].kind = PARSE_OBJECT;
                continue;
            }

            if (array_value->kind == JsonElementKind_ARRAY) {
                top++;
                stack[top].element = array_value->value.array_value;
                stack[top].kind = PARSE_ARRAY;
                continue;
            }
        }
    }

    doc->root = root_element;
    return JsonError_NONE;
}

JsonError json_doc_get_root_element(
    JsonDoc doc_handle, 
    JsonElement* out_root
) {
    JsonDoc_t* doc = (JsonDoc_t*)doc_handle;
    *out_root = doc->root;
    return JsonError_NONE;
}

JsonError json_element_get_value_object(
    JsonElement element_handle,
    JsonObject* out_object
) {
    JsonElement_t* element = (JsonElement_t*)element_handle;
    //printf("Kind: %d", element->kind);
    if (element->kind != JsonElementKind_OBJECT) {
        return JsonError_ELEMENT_KIND_MISSMATCH;
    }
    *out_object = element->value.obj_value;
    return JsonError_NONE;
}

JsonError json_object_get_property_by_name(
    JsonObject obj_handle, 
    const char* name,
    JsonElement* out_property
) {
    JsonObject_t* obj = (JsonObject_t*)obj_handle;
    for (int i = 0; i < obj->property_count; i++) {
        JsonString_t* prop_name = obj->property_names[i];
        if (strcmp(name, prop_name->bytes) == 0) {
            *out_property = obj->property_values[i];
            return JsonError_NONE;
        }
    }
    return JsonError_OBJECT_PROPERTY_NOT_FOUND;
}

JsonError json_element_get_value_string(
    JsonElement element_handle,
    JsonString* out_value
){
    JsonElement_t* element = (JsonElement_t*)element_handle;
    if (element->kind != JsonElementKind_STRING) {
        return JsonError_ELEMENT_KIND_MISSMATCH;
    }
    *out_value = element->value.str_value;
    return JsonError_NONE;
}

JsonError json_element_get_value_boolean(
    JsonElement element_handle,
    char* out_value
){
    JsonElement_t* element = (JsonElement_t*)element_handle;
    if (element->kind != JsonElementKind_BOOLEAN) {
        return JsonError_ELEMENT_KIND_MISSMATCH;
    }
    *out_value = element->value.bool_value;
    return JsonError_NONE;
}

JsonError json_element_get_value_number(
    JsonElement element_handle,
    float* out_value
){
    JsonElement_t* element = (JsonElement_t*)element_handle;
    if (element->kind != JsonElementKind_NUMBER) {
        return JsonError_ELEMENT_KIND_MISSMATCH;
    }
    *out_value = element->value.float_value;
    return JsonError_NONE;
}

JsonError json_element_get_value_array(
    JsonElement element_handle,
    JsonArray* out_value
){
    JsonElement_t* element = (JsonElement_t*)element_handle;
    if (element->kind != JsonElementKind_ARRAY) {
        return JsonError_ELEMENT_KIND_MISSMATCH;
    }
    *out_value = element->value.array_value;
    return JsonError_NONE;
}

JsonError json_array_get_length(
    JsonArray array_handle,
    size_t* out_len
) {
    JsonArray_t* array = (JsonArray_t*)array_handle;
    if (array == 0) {
        return JsonError_ARRAY_IS_NULL;
    }
    *out_len = array->elements_count;
    return JsonError_NONE;
}

JsonError json_array_get_element_at_index(
    JsonArray array_handle,
    size_t index,
    JsonElement* value
) {
    JsonArray_t* array = (JsonArray_t*)array_handle;
    if (array == 0) {
        return JsonError_ARRAY_IS_NULL;
    }
    if (index >= array->elements_count){
        return JsonError_INDEX_OUT_OF_BOUNDS;
    }

    *value = array->elements[index];
    return JsonError_NONE;
}

JsonError json_string_get_length(
    JsonString string_handle, 
    size_t* out_length
) {
    if (string_handle == 0) {
        return JsonError_STRING_IS_NULL;
    }
    JsonString_t* string = (JsonString_t*)string_handle;
    *out_length = string->length;
    return JsonError_NONE;
}

JsonError json_string_get_bytes(
    JsonString string_handle, 
    const char** out_bytes,
    size_t* out_bytes_count
) {
    JsonString_t* string = (JsonString_t*)string_handle;
    *out_bytes = string->bytes;
    *out_bytes_count = string->bytes_count;
    return JsonError_NONE;
}