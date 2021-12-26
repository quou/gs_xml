#define GS_IMPL
#define GS_XML_IMPL
#include <gs.h>
#include <gs_xml.h>

void print_indent(int indent)
{
    for (int i = 0; i < indent; i++) { putc('\t', stdout); }
}

void print_xml_node(gs_xml_node_t* node, int indent)
{
    print_indent(indent); printf("XML Node: %s\n", node->name);
    print_indent(indent); printf("\tText: %s\n", node->text);
    print_indent(indent); puts("\tAttributes:");
    for (
        gs_hash_table_iter it = gs_hash_table_iter_new(node->attributes);
        gs_hash_table_iter_valid(node->attributes, it);
        gs_hash_table_iter_advance(node->attributes, it)
    ){
        gs_xml_attribute_t attrib = gs_hash_table_iter_get(node->attributes, it);

        print_indent(indent); printf("\t\t%s: ", attrib.name);
        switch (attrib.type)
        {
            case GS_XML_ATTRIBUTE_NUMBER:
                printf("(number) %g\n", attrib.value.number);
                break;
            case GS_XML_ATTRIBUTE_BOOLEAN:
                printf("(boolean) %s\n", attrib.value.boolean ? "true" : "false");
                break;
            case GS_XML_ATTRIBUTE_STRING:
                printf("(string) %s\n", attrib.value.string);
                break;
            default: break; // Unreachable
        }
    }

    if (gs_dyn_array_size(node->children) > 0)
    {
         print_indent(indent); printf("\t = Children = \n");
         for (uint32_t i = 0; i < gs_dyn_array_size(node->children); i++)
         {
             print_xml_node(node->children + i, indent + 1);
         }
    } 
}

void init()
{
    gs_xml_document_t* doc = gs_xml_parse_file("./test.xml");
    if (!doc)
    {
        printf("XML Parse Error: %s\n", gs_xml_get_error());
        return;
    }

    for (uint32_t i = 0; i < gs_dyn_array_size(doc->nodes); i++)
    {
        gs_xml_node_t* node = doc->nodes + i;

        print_xml_node(node, 0);
    }

    gs_xml_free(doc);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .init = init,
    };
}
