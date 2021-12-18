#ifndef GS_XML_H
#define GS_XML_H

#include "gs.h"

typedef enum gs_xml_attribute_type_t {
    GS_XML_ATTRIBUTE_INT,
    GS_XML_ATTRIBUTE_UINT,
    GS_XML_ATTRIBUTE_STRING,
    GS_XML_ATTRIBUTE_FLOAT
} gs_xml_attribute_type_t;

typedef struct gs_xml_attribute_t {
    char* name;
    gs_xml_attribute_type_t type;

    union {
        int32_t  i;
        uint32_t u;
        char*    string;
        double   f;
    } value;
} gs_xml_attribute_t;

typedef struct gs_xml_node_t {
	char* name;
	char* text;

	gs_dyn_array(gs_xml_attribute_t) attributes;

    gs_dyn_array(struct gs_xml_node_t) children;
} gs_xml_node_t;

typedef struct gs_xml_document_t {
    gs_dyn_array(gs_xml_node_t) nodes;
} gs_xml_document_t;

gs_xml_document_t* gs_xml_parse(const char* source);
gs_xml_document_t* gs_xml_parse_file(const char* path);
void gs_xml_free(gs_xml_document_t* document);

#ifdef GS_XML_IMPL

gs_xml_document_t* gs_xml_parse_file(const char* path)
{
    gs_xml_document_t* doc = gs_xml_parse(source);

    gs_free(source);

    return doc;
}

gs_xml_document_t* gs_xml_parse(const char* source)
{
    gs_xml_document_t* doc = gs_calloc(1, sizeof(gs_xml_document_t));

    return doc;
}

void gs_xml_free(gs_xml_document_t* document)
{
    gs_dyn_array_free(documebnt->children);
    gs_free(document);
}

#undef GS_XML_IMPL
#endif // GS_XML_IMPL
#endif // GS_XML_H
