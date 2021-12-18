#define GS_IMPL
#define GS_XML_IMPL
#include <gs.h>
#include <gs_xml.h>

void init()
{
    gs_xml_document_t* doc = gs_xml_parse_file("./test.xml");

    gs_xml_free(doc);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .init = init,
    };
}
