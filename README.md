# Gunslinger XML
A lightweight XML parser for use with [Gunslinger](https://github.com/MrFrenik/gunslinger).

At the moment, this library is capable of parsing the following document:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<test some_attrib="I'm an attribute!" some_entities="&apos; &amp; &quot; &lt; &gt;" some_number1  =     "384.7" some_bool="true">
	<test_child>Hello, world!</test_child>
	<other_test_child some_bool="false">
		<other_test_child_child>Hello there! &apos; &amp; &quot; &lt; &gt;</other_test_child_child>
	</other_test_child>

	<!-- Hello, I'm a comment. -->

	<some_thing thing="Haha yes" thing1 = "33"/>

	<!-- I'm another comment. -->
</test>
```

It supports the following entities:
| Character | Entity |
| --- | --- |
| & | `&amp;` |
| ' | `&apos;` |
| " | `&quot;` |
| < | `&lt;` |
| > | `&gt;` |

## Usage
Before use, make sure you have Gunslinger set up, including adding `gs.h`, from the
[Gunslinger repository](https://github.com/MrFrenik/gunslinger) to your include path.
Then, `#define GS_XML_IMPL` in *one* C or C++ source file before you `#include gs_xml.h`.

### Basic Usage
```c
// To parse from memory: gs_xml_parse(const char* source)
gs_xml_document_t* doc = gs_xml_parse_file("./test.xml");
if (!doc)
{
	printf("XML Parse Error: %s\n", gs_xml_get_error());
	return;
}

gs_xml_free(doc);
```

## Examples
A more complete example can be found in [example](https://github.com/veridisquot/gs_xml/tree/master/example).

### Finding Elements
```c
// To find an element that isn't a child
gs_xml_node_t* node = gs_xml_find_node(doc, "test_node");

// To find a child
gs_xml_node_t* child = gs_xml_find_node(node, "test_child");

// To find an attribute
gs_xml_attribute_t* attrib = gs_xml_find_attribute(node, "test_attrib");
```

### Getting the Name of Nodes
```c
printf("%s\n", node->name);
```

### Getting the Name & Value of attributes
```c
printf("%s\n", attrib->name);

printf("%g\n", number_attrib->value.number);
printf("%s\n", number_attrib->value.string);
printf("%s\n", number_attrib->value.boolean ? "true" : "false");
```
