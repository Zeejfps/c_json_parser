#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "json.h"

int main() {
    FILE* file = fopen("simple.json", "r");
    if (file == 0){
        printf("Failed to open file\n");
        return 1;
    }

    JsonDoc doc;
    json_doc_create(&doc);

    JsonError result = json_parse_file(doc, file);

    if (result != JsonError_NONE) {
        printf("Error parsing file: %d", result);
        json_doc_destroy(&doc);
        return 1;
    }

    JsonElement root;
    json_doc_get_root_element(doc, &root);

    JsonObject rootObj;
    json_element_get_object_value(root, &rootObj);

    JsonElement nameProp;
    json_object_get_property_by_name(rootObj, "name", &nameProp);

    

    json_doc_destroy(&doc);

    // JsonElement child;
    // json_element_create(&child, doc);

    // json_get_element_child_by_name(doc, root, "name", &child);

    // u_int32_t len;
    // json_get_element_value_str_len(doc, child, &len);

    // char* name = (char*)calloc(len + 1, sizeof(char));
    // json_get_element_value_str(doc, child, name);

    // json_element_destroy(root);

    // json_doc_destroy(doc);

    return 0;
}