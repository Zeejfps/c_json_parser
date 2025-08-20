#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "json.h"

int main() {
    FILE* stream = fopen("sample.json", "r");
    if (stream == 0){
        printf("Failed to open file\n");
        return 1;
    }

    JsonParser parser;
    json_parser_create(&parser);

    JsonDoc doc;
    json_doc_create(&doc);

    json_parse_file(parser, doc, stream);
    
    json_parser_destroy(&parser);

    JsonElement root;
    json_element_create(&root, doc);
    
    json_get_root_element(doc, &root);

    JsonElement child;
    json_element_create(&child, doc);

    json_get_element_child_by_name(doc, root, "name", &child);

    u_int32_t len;
    json_get_element_value_str_len(doc, child, &len);

    char* name = (char*)calloc(len + 1, sizeof(char));
    json_get_element_value_str(doc, child, name);

    json_element_destroy(root);

    json_doc_destroy(doc);

    return 0;
}