int main() {

    JsonParser parser = json_parser_create();

    JsonDoc doc = json_doc_create();

    json_parse_file(parser, doc, file);

    JsonElement root = json_element_create();
    
    json_get_root_element(doc, root);

    JsonElement child = json_element_create();

    json_get_element_child_by_name(root, "name", child);

    json_get_element_value_str(child, "aasd");

    json_element_destroy(root);

    json_doc_destroy(doc);

    json_parser_destroy(parser);

    return 0;
}