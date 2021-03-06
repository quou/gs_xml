#ifndef GS_XML_H
#define GS_XML_H

#include "gs.h"

typedef enum gs_xml_attribute_type_t
{
    GS_XML_ATTRIBUTE_NUMBER,
    GS_XML_ATTRIBUTE_BOOLEAN,
    GS_XML_ATTRIBUTE_STRING,
} gs_xml_attribute_type_t;

typedef struct gs_xml_attribute_t
{
    char* name;
    gs_xml_attribute_type_t type;

    union
    {
        double   number;
        bool     boolean;
        char*    string;
    } value;
} gs_xml_attribute_t;

typedef struct gs_xml_node_t
{
	char* name;
	char* text;

	gs_hash_table(uint64_t, gs_xml_attribute_t) attributes;

    gs_dyn_array(struct gs_xml_node_t) children;
} gs_xml_node_t;

typedef struct gs_xml_document_t
{
    gs_dyn_array(gs_xml_node_t) nodes;
} gs_xml_document_t;

typedef struct gs_xml_node_iter_t
{
    gs_xml_document_t* doc;
    gs_xml_node_t* node;
    const char* name;
    uint32_t idx;

    gs_xml_node_t* current;
} gs_xml_node_iter_t;

GS_API_DECL gs_xml_document_t* gs_xml_parse(const char* source);
GS_API_DECL gs_xml_document_t* gs_xml_parse_file(const char* path);
GS_API_DECL void gs_xml_free(gs_xml_document_t* document);

GS_API_DECL gs_xml_attribute_t* gs_xml_find_attribute(gs_xml_node_t* node, const char* name);
GS_API_DECL gs_xml_node_t* gs_xml_find_node(gs_xml_document_t* doc, const char* name);
GS_API_DECL gs_xml_node_t* gs_xml_find_node_child(gs_xml_node_t* node, const char* name);

GS_API_DECL const char* gs_xml_get_error();

GS_API_DECL gs_xml_node_iter_t gs_xml_new_node_iter(gs_xml_document_t* doc, const char* name);
GS_API_DECL gs_xml_node_iter_t gs_xml_new_node_child_iter(gs_xml_node_t* node, const char* name);
GS_API_DECL bool gs_xml_node_iter_next(gs_xml_node_iter_t* iter);

#ifdef GS_XML_IMPL

#define GS_XML_EXPECT(c_, e_) \
    if (*c_ != e_) \
    { \
        gs_xml_emit_error("Expected " #e_ "."); \
        return NULL; \
    }

#define GS_XML_EXPECT_NOT_END(c_) \
    if (!*(c_)) \
    { \
        gs_xml_emit_error("Unexpected end."); \
        return NULL; \
    }

typedef struct gs_xml_entity_t
{
    char character;
    const char* name;
} gs_xml_entity_t;

static gs_xml_entity_t gs_xml_entities[] =
{
    { '&',  "&amp;" },
    { '\'', "&apos;" },
    { '"',  "&quot;" },
    { '<',  "&lt;" },
    { '>',  "&gt;" }
};

static const char* gs_xml_error = NULL;

static void gs_xml_emit_error(const char* error)
{
    gs_xml_error = error;
}

static char* gs_xml_copy_string(const char* str, uint32_t len)
{
    char* r = gs_malloc(len + 1);
    if (!r) { gs_xml_emit_error("Out of memory!"); return NULL; }
    r[len] = '\0';
    
    for (uint32_t i = 0; i < len; i++)
    {
        r[i] = str[i];
    }

    return r;
}

static bool gs_xml_string_is_decimal(const char* str, uint32_t len)
{
    uint32_t i = 0;
    if (str[0] == '-') i++;

    bool used_dot = false;

    for (; i < len; i++)
    {
        char c = str[i];
        if (c < '0' || c > '9')
        {
            if (c == '.' && !used_dot)
            {
                used_dot = true;
                continue;
            }
            return false;
        }
    }

    return true;
}

static bool gs_xml_string_equal(const char* str_a, uint32_t len, const char* str_b) {
    for (uint32_t i = 0; i < len; i++) {
        if (str_a[i] != str_b[i]) return false;
    }

    return true;
}

static uint64_t gs_xml_hash_string(const char* str, uint32_t len)
{
	uint64_t hash = 0, x = 0;

	for (uint32_t i = 0; i < len; i++) {
		hash = (hash << 4) + str[i];
		if ((x = hash & 0xF000000000LL) != 0) {
			hash ^= (x >> 24);
			hash &= ~x;
		}
	}

	return (hash & 0x7FFFFFFFFF);
}

static void gs_xml_node_free(gs_xml_node_t* node)
{
    for (
        gs_hash_table_iter it = gs_hash_table_iter_new(node->attributes);
        gs_hash_table_iter_valid(node->attributes, it);
        gs_hash_table_iter_advance(node->attributes, it)
    )
    {
        gs_xml_attribute_t attrib = gs_hash_table_iter_get(node->attributes, it);

        if (attrib.type == GS_XML_ATTRIBUTE_STRING)
        {
			gs_free(attrib.value.string);
        }
    }

    for (uint32_t i = 0; i < gs_dyn_array_size(node->children); i++)
    {
        gs_xml_node_free(node->children + i);
    }

    gs_free(node->name);
    gs_free(node->text);
    gs_hash_table_free(node->attributes);
    gs_dyn_array_free(node->children);
}

static char* gs_xml_process_text(const char* start, uint32_t length)
{
    char* r = gs_malloc(length + 1);
    
    uint32_t len_sub = 0;

    for (uint32_t i = 0, ri = 0; i < length; i++, ri++)
    {
        bool changed = false;
        if (start[i] == '&')
        {
        	for (uint32_t ii = 0; ii < sizeof(gs_xml_entities) / sizeof(*gs_xml_entities); ii++)
        	{
            	uint32_t ent_len = gs_string_length(gs_xml_entities[ii].name);
            	if (gs_xml_string_equal(start + i, ent_len, gs_xml_entities[ii].name))
            	{
                	r[ri] = gs_xml_entities[ii].character;
                	i += ent_len - 1;
                	len_sub += ent_len - 1;
                	changed = true;
                	break;
            	}
        	}
        }

        if (!changed)
            r[ri] = start[i];
    }

    r[length - len_sub] = '\0';

    return r;
}

// Parse an XML block. Returns an array of nodes in the block.
static gs_dyn_array(gs_xml_node_t) gs_xml_parse_block(const char* start, uint32_t length)
{
    gs_dyn_array(gs_xml_node_t) r = NULL;

    bool inside = false;

    for (const char* c = start; *c && c < start + length; c++)
    {
        if (*c == '<')
        {
            c++;
            GS_XML_EXPECT_NOT_END(c);
            
            if (*c == '?') // Skip the XML header.
            {
                c++;
                GS_XML_EXPECT_NOT_END(c);
                while (*c != '>')
                {
                    c++;
                    GS_XML_EXPECT_NOT_END(c);
                }
                continue;
            }
            else if (gs_xml_string_equal(c, 3, "!--")) // Skip comments.
            {
                c++; GS_XML_EXPECT_NOT_END(c);
                c++; GS_XML_EXPECT_NOT_END(c);
                c++; GS_XML_EXPECT_NOT_END(c);
                while (!gs_xml_string_equal(c, 3, "-->"))
                {
                    c++; GS_XML_EXPECT_NOT_END(c);
                }

                continue;
            }

            if (inside && *c == '/')
                inside = false;
            else
                inside = true;

            const char* node_name_start = c;
            uint32_t node_name_len = 0;

            gs_xml_node_t cur_node = { 0 };

            if (inside)
            {
            	for (; *c != '>' && *c != ' ' && *c != '/'; c++)
                    node_name_len++;

                if (*c != '>')
                {
                    while (*c != '>' && *c != '/')
                    {
                        while (gs_char_is_white_space(*c)) c++;

                        const char* attrib_name_start = c;
                        uint32_t attrib_name_len = 0;

                        while (gs_char_is_alpha(*c) || gs_char_is_numeric(*c) || *c == '_')
                        {
                            c++;
                            attrib_name_len++;
                            GS_XML_EXPECT_NOT_END(c);
                        }

                        while (*c != '"')
                        {
                            c++;
                            GS_XML_EXPECT_NOT_END(c);
                        }

                        c++;
                        GS_XML_EXPECT_NOT_END(c);

                        const char* attrib_text_start = c;
                        uint32_t attrib_text_len = 0;

                        while (*c != '"')
                        {
                            c++;
                            attrib_text_len++;
                            GS_XML_EXPECT_NOT_END(c);
                        }

                        c++;
                        GS_XML_EXPECT_NOT_END(c);

                        gs_xml_attribute_t attrib = { 0 };
                        attrib.name = gs_xml_copy_string(attrib_name_start, attrib_name_len);

                        if (gs_xml_string_is_decimal(attrib_text_start, attrib_text_len))
                        {
                            attrib.type = GS_XML_ATTRIBUTE_NUMBER;
                            attrib.value.number = strtod(attrib_text_start, NULL);
                        }
                        else if (gs_xml_string_equal(attrib_text_start, attrib_text_len, "true"))
                        {
                            attrib.type = GS_XML_ATTRIBUTE_BOOLEAN;
                            attrib.value.boolean = true;
                        }
                        else if (gs_xml_string_equal(attrib_text_start, attrib_text_len, "false"))
                        {
                            attrib.type = GS_XML_ATTRIBUTE_BOOLEAN;
                            attrib.value.boolean = false;
                        }
                        else
                        {
                            attrib.type = GS_XML_ATTRIBUTE_STRING;
                            attrib.value.string = gs_xml_process_text(attrib_text_start, attrib_text_len);
                        }

                        gs_hash_table_insert(cur_node.attributes,
                            gs_xml_hash_string(attrib_name_start, attrib_name_len),
                            attrib);
                    }
                }

                if (*c == '/') // For nodes without any text.
                {
                    c++;
                    GS_XML_EXPECT_NOT_END(c);
                    cur_node.name = gs_xml_copy_string(node_name_start, node_name_len);
                    gs_dyn_array_push(r, cur_node);
                    inside = false;
                }
            }
            else
            {
                while (*c != '>') { c++; GS_XML_EXPECT_NOT_END(c); }
            }

            c++;
            GS_XML_EXPECT_NOT_END(c);

            if (inside)
            {
                const char* text_start = c;
                uint32_t text_len = 0;

                const char* end_start = c;
                uint32_t end_len = 0;

                cur_node.name = gs_xml_copy_string(node_name_start, node_name_len);

                for (uint32_t i = 0; i < length; i++)
                {
                    if (*c == '<' && *(c + 1) == '/')
                    {
                        c++; GS_XML_EXPECT_NOT_END(c);
                        c++; GS_XML_EXPECT_NOT_END(c);
                        end_start = c;
                        end_len = 0;
                        while (*c != '>') {
                            end_len++;
                            c++;
                            GS_XML_EXPECT_NOT_END(c);
                        }

                        if (gs_xml_string_equal(end_start, end_len, cur_node.name))
                        {
                            break;
                        }
                        else
                        {
                            text_len += end_len + 2;
                            continue;
                        }
                    }

                    c++;
                    text_len++;

                    GS_XML_EXPECT_NOT_END(c);
                }

                cur_node.children = gs_xml_parse_block(text_start, text_len);
                if (gs_dyn_array_size(cur_node.children) == 0)
                    cur_node.text = gs_xml_process_text(text_start, text_len);
                else
                    cur_node.text = gs_xml_copy_string(text_start, text_len);

                gs_dyn_array_push(r, cur_node);

                c--;
            }
        }
    }

    return r;
}

gs_xml_document_t* gs_xml_parse_file(const char* path)
{
    size_t size;
    char* source = gs_read_file_contents_into_string_null_term(path, "r", &size);

    if (!source)
    {
        gs_xml_emit_error("Failed to read file!");
        return NULL;
    }

    gs_xml_document_t* doc = gs_xml_parse(source);

    gs_free(source);

    return doc;
}

gs_xml_document_t* gs_xml_parse(const char* source)
{
    if (!source) return NULL;

    gs_xml_error = NULL;
    gs_xml_document_t* doc = gs_calloc(1, sizeof(gs_xml_document_t));
    if (!doc) { gs_xml_emit_error("Out of memory!"); return NULL; }

    doc->nodes = gs_xml_parse_block(source, gs_string_length(source));

    if (gs_xml_error)
    {
        gs_xml_free(doc);
        return NULL;
    }

    return doc;
}

void gs_xml_free(gs_xml_document_t* document)
{
    for (uint32_t i = 0; i < gs_dyn_array_size(document->nodes); i++)
    {
        gs_xml_node_free(document->nodes + i);
    }

    gs_dyn_array_free(document->nodes);
    gs_free(document);
}

gs_xml_attribute_t* gs_xml_find_attribute(gs_xml_node_t* node, const char* name)
{
	if (!gs_hash_table_exists(node->attributes, gs_xml_hash_string(name, gs_string_length(name)))) {
		return NULL;
	} else {
    	return gs_hash_table_getp(node->attributes, gs_xml_hash_string(name, gs_string_length(name)));
    }
}

gs_xml_node_t* gs_xml_find_node(gs_xml_document_t* doc, const char* name)
{
    for (uint32_t i = 0; i < gs_dyn_array_size(doc->nodes); i++)
    {
        if (gs_string_compare_equal(name, doc->nodes[i].name))
        {
            return doc->nodes + i;
        }
    }

    return NULL;
}

gs_xml_node_t* gs_xml_find_node_child(gs_xml_node_t* node, const char* name)
{
    for (uint32_t i = 0; i < gs_dyn_array_size(node->children); i++)
    {
        if (gs_string_compare_equal(name, node->children[i].name))
        {
            return node->children + i;
        }
    }

    return NULL;
}

const char* gs_xml_get_error()
{
    return gs_xml_error;
}

GS_API_DECL gs_xml_node_iter_t gs_xml_new_node_iter(gs_xml_document_t* doc, const char* name)
{
    gs_xml_node_iter_t it = {
        .doc = doc,
        .name = name,
        .idx = 0
    };

    return it;
}

GS_API_DECL gs_xml_node_iter_t gs_xml_new_node_child_iter(gs_xml_node_t* parent, const char* name)
{
    gs_xml_node_iter_t it = {
        .node = parent,
        .name = name,
        .idx = 0
    };

    return it;
}

GS_API_DECL bool gs_xml_node_iter_next(gs_xml_node_iter_t* iter)
{
    if (iter->node)
    {
        for (uint32_t i = iter->idx; i < gs_dyn_array_size(iter->node->children); i++) {
            if (gs_string_compare_equal(iter->name, iter->node->children[i].name))
            {
                iter->current = iter->node->children + i;
                iter->idx = i + 1;
                return true;
            }
        }

        return false;
    }
    else
    {
        for (uint32_t i = iter->idx; i < gs_dyn_array_size(iter->doc->nodes); i++) {
            if (gs_string_compare_equal(iter->name, iter->doc->nodes[i].name))
            {
                iter->current = iter->doc->nodes + i;
                iter->idx = i + 1;
                return true;
            }
        }
        return false;
    }
}

#undef GS_XML_IMPL
#endif // GS_XML_IMPL
#endif // GS_XML_H
