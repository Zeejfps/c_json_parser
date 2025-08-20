#include <stdio.h>
#include <stdint.h>
#include "json.h"

int main() {
    FILE* stream = fopen("settings.json", "r");
    if (stream == 0){
        printf("Failed to open file\n");
        return 1;
    }

    Json_Parser parser = json_parser_create();
    Json_Parse_Result result = json_parser_parse_file(parser, stream);
    if (result.error != NONE) {
        printf('Failed to parse json\n');
        json_parser_destroy(parser);
        return 1;
    }
    Json_Doc doc = result.doc;

    Json_Element root = json_doc_get_root(doc);
    Json_Get_Element_Child_By_Name_Result get_child_by_name_result =
        json_get_element_child_by_name(doc, root, "name");

    if (get_child_by_name_result.error != NONE) {
        printf("Failed to get child with name %s\n", "name");
        json_parser_destroy(parser);
        return 1;
    }

    Json_Get_Element_Value_Str_Len_Result get_element_value_str_len_result = 
        json_get_element_value_str_len(doc, get_child_by_name_result.element);
    if (get_element_value_str_len_result.error != NONE) {
        printf("Failed to get element value string length");
        json_parser_destroy(parser);
        return 1;
    }

    char* element_value = (char*)calloc(get_element_value_str_len_result.length + 1, sizeof(char));

    json_parser_destroy(parser);

    return 0;
}