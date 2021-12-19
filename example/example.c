#define GS_IMPL
#define GS_XML_IMPL
#include <gs.h>
#include <gs_xml.h>

void init()
{
    gs_xml_document_t* doc = gs_xml_parse_file("./test.xml");

    for (uint32_t i = 0; i < gs_dyn_array_size(doc->nodes); i++)
    {
        gs_xml_node_t* node = doc->nodes + i;

        printf("XML Node: %s\n", doc->nodes[i].name);
        puts("\tChildren:");
        for (
            gs_hash_table_iter it = gs_hash_table_iter_new(node->attributes);
            gs_hash_table_iter_valid(node->attributes, it);
            gs_hash_table_iter_advance(node->attributes, it)
        ){
            gs_xml_attribute_t attrib = gs_hash_table_iter_get(node->attributes, it);

            printf("\t\t%s: ", attrib.name);
            switch (attrib.type)
            {
                case GS_XML_ATTRIBUTE_NUMBER:
                    printf("(number) %g\n", attrib.value.number);
                    break;
                case GS_XML_ATTRIBUTE_BOOLEAN:
                    printf("(boolean) %d\n", attrib.value.boolean);
                    break;
                case GS_XML_ATTRIBUTE_STRING:
                    printf("(string) %s\n", attrib.value.string);
                    break;
                default: break; // Unreachable
            }
        }
    }

    gs_xml_free(doc);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .init = init,
    };
}
