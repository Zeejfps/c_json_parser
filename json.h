#ifndef PARSER_H
#define PARSER_H

#include<stdio.h>
#include <stdint.h>

typedef void* Json_Parser;
typedef void* Json_Doc;
typedef void* Json_Element;

typedef struct Json_Parse_Result {
    enum Json_Parse_Error { NONE, UNKNOWN_ERROR } error;
    Json_Parse_Error error;
    Json_Doc doc;
} Json_Parse_Result;

typedef struct Json_Get_Element_Child_By_Name_Result {
    enum Json_Get_Child_By_Name_Error { NONE, UNKNOWN_ERROR };
    Json_Get_Child_By_Name_Error error;
    Json_Element element;
} Json_Get_Element_Child_By_Name_Result;

typedef struct Json_Get_Element_Value_Str_Len_Result
{
    enum Json_Get_Child_By_Name_Error { NONE, UNKNOWN_ERROR, ELEMENT_NOT_STRING };
    Json_Get_Child_By_Name_Error error;
    u_int32_t length;
} Json_Get_Element_Value_Str_Len_Result;

typedef struct Json_Get_Element_Value_Str_Result
{
    enum Json_Get_Child_By_Name_Error { NONE, UNKNOWN_ERROR, ELEMENT_NOT_STRING };
    Json_Get_Child_By_Name_Error error;
};


Json_Parser json_parser_create();
void json_parser_destroy(Json_Parser parser);
Json_Parse_Result json_parser_parse_file(Json_Parser parser, FILE* file);

Json_Element 
json_doc_get_root(
    Json_Doc doc
);

Json_Get_Element_Child_By_Name_Result 
json_get_element_child_by_name(
    Json_Doc doc, 
    Json_Element parent, 
    const char* name
);

Json_Get_Element_Value_Str_Len_Result
json_get_element_value_str_len(
    Json_Doc doc, 
    Json_Element element
);

Json_Get_Element_Value_Str_Result
json_get_element_value_str(
    Json_Doc doc, 
    Json_Element element,
    const char* buffer
);

#endif