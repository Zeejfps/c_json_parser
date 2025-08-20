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

    JsonObject root_obj;
    json_element_get_value_object(root, &root_obj);

    JsonElement name_prop;
    json_object_get_property_by_name(root_obj, "name", &name_prop);

    if (name_prop == 0) {
        printf("Failed to find property with name: %s", "name");
    }

    char* name_prop_value;
    json_element_get_value_string(name_prop, &name_prop_value);

    printf("Name Prop Value: '%s'\n", name_prop_value);

    JsonElement tags_prop;
    json_object_get_property_by_name(root_obj, "tags", &tags_prop);

    JsonArray tags_array;
    json_element_get_value_array(tags_prop, &tags_array);

    size_t tags_count;
    json_array_get_length(tags_array, &tags_count);

    printf("Tags count: %zu\n", tags_count);

    JsonElement tag_element;
    json_array_get_element_at_index(tags_array, 1, &tag_element);

    char* tag_value;
    json_element_get_value_string(tag_element, &tag_value);

    printf("Tag value: '%s'\n", tag_value);

    json_doc_destroy(&doc);

    return 0;
}