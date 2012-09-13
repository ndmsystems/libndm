#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ndm/xml.h>
#include "test.h"

#define STATIC_BUFFER_SIZE			4096
#define DYNAMIC_BUFFER_SIZE			4096

int main()
{
	char buffer[STATIC_BUFFER_SIZE];
	struct ndm_xml_document_t d = NDM_XML_DOCUMENT_INITIALIZER(
		buffer, sizeof(buffer), DYNAMIC_BUFFER_SIZE);
	char cbuffer[STATIC_BUFFER_SIZE];
	struct ndm_xml_document_t copy =
		NDM_XML_DOCUMENT_INITIALIZER(
			cbuffer, sizeof(cbuffer), DYNAMIC_BUFFER_SIZE);
	struct ndm_xml_node_t *root = NULL;
	struct ndm_xml_node_t *n = NULL;
	struct ndm_xml_node_t *c = NULL;
	struct ndm_xml_attr_t *a = NULL;
	struct ndm_xml_node_t *p[6];
	struct ndm_xml_attr_t *q[6];
	char *s = NULL;
	FILE *fp = NULL;
	long fsize = 0;

	NDM_TEST_BREAK_IF((root = ndm_xml_document_alloc_root(&d)) == NULL);

	NDM_TEST(*ndm_xml_node_name(root) == '\0');
	NDM_TEST(*ndm_xml_node_value(root) == '\0');
	NDM_TEST(ndm_xml_node_type(root) == NDM_XML_NODE_TYPE_DOCUMENT);
	NDM_TEST(ndm_xml_node_first_child(root, NULL) == NULL);
	NDM_TEST(ndm_xml_node_first_attr(root, NULL) == NULL);

	ndm_xml_node_set_name(root, "test");

	NDM_TEST(strcmp(ndm_xml_node_name(root), "test") == 0);

	ndm_xml_node_set_value(root, "test_value");

	NDM_TEST(strcmp(ndm_xml_node_value(root), "test_value") == 0);

	NDM_TEST(ndm_xml_node_parent(root) == NULL);

	n = ndm_xml_document_alloc_node(&d,
		NDM_XML_NODE_TYPE_ELEMENT, NULL, NULL);

	NDM_TEST_BREAK_IF(n == NULL);
	NDM_TEST(*ndm_xml_node_name(n) == '\0');
	NDM_TEST(*ndm_xml_node_value(n) == '\0');

	NDM_TEST_BREAK_IF((n = ndm_xml_document_alloc_node(
		&d, NDM_XML_NODE_TYPE_ELEMENT, "test", NULL)) == NULL);

	NDM_TEST(strcmp(ndm_xml_node_name(n), "test") == 0);
	NDM_TEST(*ndm_xml_node_value(n) == '\0');

	NDM_TEST_BREAK_IF((n = ndm_xml_document_alloc_node(
		&d, NDM_XML_NODE_TYPE_ELEMENT, NULL, "test_value")) == NULL);

	NDM_TEST(ndm_xml_node_type(n) == NDM_XML_NODE_TYPE_ELEMENT);
	NDM_TEST(*ndm_xml_node_name(n) == '\0');
	NDM_TEST(strcmp(ndm_xml_node_value(n), "test_value") == 0);

	NDM_TEST_BREAK_IF(
		(a = ndm_xml_document_alloc_attr(&d, NULL, NULL)) == NULL);

	NDM_TEST(*ndm_xml_attr_name(a) == '\0');
	NDM_TEST(*ndm_xml_attr_value(a) == '\0');

	NDM_TEST_BREAK_IF(
		(a = ndm_xml_document_alloc_attr(&d, "test", NULL)) == NULL);

	NDM_TEST(strcmp(ndm_xml_attr_name(a), "test") == 0);
	NDM_TEST(*ndm_xml_attr_value(a) == '\0');

	NDM_TEST_BREAK_IF(
		(a = ndm_xml_document_alloc_attr(&d, NULL, "test_value")) == NULL);

	NDM_TEST(*ndm_xml_attr_name(a) == '\0');
	NDM_TEST(strcmp(ndm_xml_attr_value(a), "test_value") == 0);

	NDM_TEST(ndm_xml_node_parent(n) == NULL);
	NDM_TEST(ndm_xml_attr_node(a) == NULL);

	ndm_xml_node_append_child(root, n);
	ndm_xml_node_append_attr(n, a);

	NDM_TEST(ndm_xml_node_parent(n) == root);
	NDM_TEST(ndm_xml_attr_node(a) == n);

	NDM_TEST_BREAK_IF((s = ndm_xml_document_alloc_str(&d, "")) == NULL);

	NDM_TEST(*s == '\0');

	NDM_TEST_BREAK_IF(
		(c = ndm_xml_node_append_child_str(n, "child", NULL)) == NULL);

	NDM_TEST(strcmp(ndm_xml_node_name(c), "child") == 0);
	NDM_TEST(*ndm_xml_node_value(c) == '\0');
	NDM_TEST(ndm_xml_node_parent(c) == n);

	NDM_TEST_BREAK_IF(
		(c = ndm_xml_node_append_child_str(n, "name", "value")) == NULL);

	NDM_TEST(strcmp(ndm_xml_node_name(c), "name") == 0);
	NDM_TEST(strcmp(ndm_xml_node_value(c), "value") == 0);
	NDM_TEST(ndm_xml_node_parent(c) == n);

	NDM_TEST_BREAK_IF(
		(c = ndm_xml_node_append_child_int(n, "name2", -100)) == NULL);

	NDM_TEST(strcmp(ndm_xml_node_name(c), "name2") == 0);
	NDM_TEST(strcmp(ndm_xml_node_value(c), "-100") == 0);
	NDM_TEST(ndm_xml_node_parent(c) == n);

	NDM_TEST_BREAK_IF(
		(c = ndm_xml_node_append_child_int(n, "name3", 9999999)) == NULL);

	NDM_TEST(strcmp(ndm_xml_node_name(c), "name3") == 0);
	NDM_TEST(strcmp(ndm_xml_node_value(c), "9999999") == 0);
	NDM_TEST(ndm_xml_node_parent(c) == n);

	NDM_TEST_BREAK_IF(
		(a = ndm_xml_node_append_attr_str(n, "child", NULL)) == NULL);

	NDM_TEST(strcmp(ndm_xml_attr_name(a), "child") == 0);
	NDM_TEST(strcmp(ndm_xml_attr_value(a), "") == 0);
	NDM_TEST(ndm_xml_attr_node(a) == n);

	NDM_TEST_BREAK_IF(
		(a = ndm_xml_node_append_attr_int(n, "child2", -1000)) == NULL);

	NDM_TEST(strcmp(ndm_xml_attr_name(a), "child2") == 0);
	NDM_TEST(strcmp(ndm_xml_attr_value(a), "-1000") == 0);
	NDM_TEST(ndm_xml_attr_node(a) == n);

	NDM_TEST_BREAK_IF(
		(n = ndm_xml_node_append_child_str(root, "node", NULL)) == NULL);

	NDM_TEST(ndm_xml_node_first_child(n, "child1") == NULL);
	NDM_TEST_BREAK_IF(
		(p[0] = ndm_xml_node_append_child_str(n, "child1", NULL)) == NULL);
	NDM_TEST_BREAK_IF(
		(p[1] = ndm_xml_node_append_child_str(n, "child2", NULL)) == NULL);
	NDM_TEST_BREAK_IF(
		(p[2] = ndm_xml_node_append_child_str(n, "child3", NULL)) == NULL);
	NDM_TEST_BREAK_IF(
		(p[3] = ndm_xml_node_append_child_str(n, "child1", NULL)) == NULL);
	NDM_TEST_BREAK_IF(
		(p[4] = ndm_xml_node_append_child_str(n, "child4", NULL)) == NULL);
	NDM_TEST_BREAK_IF(
		(p[5] = ndm_xml_node_append_child_str(n, "child1", NULL)) == NULL);

	NDM_TEST(ndm_xml_node_first_child(n, "child1") == p[0]);
	NDM_TEST(ndm_xml_node_last_child(n, NULL) == p[5]);
	NDM_TEST(ndm_xml_node_last_child(n, "child12") == NULL);
	NDM_TEST(ndm_xml_node_last_child(n, "child1") == p[5]);
	NDM_TEST(
		ndm_xml_node_last_child(n, "child2") ==
		ndm_xml_node_first_child(n, "child2"));
	NDM_TEST(
		ndm_xml_node_last_child(n, "child3") ==
		ndm_xml_node_first_child(n, "child3"));
	NDM_TEST(
		ndm_xml_node_last_child(n, "child4") ==
		ndm_xml_node_first_child(n, "child4"));
	NDM_TEST(ndm_xml_node_prev_sibling(p[0], NULL) == NULL);
	NDM_TEST(ndm_xml_node_next_sibling(p[5], NULL) == NULL);
	NDM_TEST(ndm_xml_node_next_sibling(p[0], NULL) == p[1]);
	NDM_TEST(ndm_xml_node_prev_sibling(p[1], NULL) == p[0]);
	NDM_TEST(ndm_xml_node_next_sibling(p[2], "child1") == p[3]);
	NDM_TEST(ndm_xml_node_next_sibling(p[2], "child4") == p[4]);

	ndm_xml_node_remove_first_child(n);
	ndm_xml_node_remove_last_child(n);

	NDM_TEST(ndm_xml_node_prev_sibling(p[1], NULL) == NULL);
	NDM_TEST(ndm_xml_node_next_sibling(p[4], NULL) == NULL);

	ndm_xml_node_prepend_child(n, p[0]);
	ndm_xml_node_append_child(n, p[5]);

	NDM_TEST(ndm_xml_node_first_child(n, NULL) == p[0]);
	NDM_TEST(ndm_xml_node_last_child(n, NULL) == p[5]);

	NDM_TEST(ndm_xml_node_first_child(n, "child3") == p[2]);
	NDM_TEST(ndm_xml_node_last_child(n, "child3") == p[2]);

	ndm_xml_node_remove_child(n, p[2]);

	NDM_TEST(ndm_xml_node_first_child(n, "child3") == NULL);
	NDM_TEST(ndm_xml_node_last_child(n, "child3") == NULL);

	ndm_xml_node_insert_child(n, p[1], p[2]);

	NDM_TEST(ndm_xml_node_first_child(n, "child3") == p[2]);
	NDM_TEST(ndm_xml_node_last_child(n, "child3") == p[2]);

	ndm_xml_node_remove_all_children(n, NULL);

	NDM_TEST(ndm_xml_node_first_child(n, NULL) == NULL);

	NDM_TEST(ndm_xml_node_first_attr(n, "attr1") == NULL);

	NDM_TEST_BREAK_IF(
		(q[0] = ndm_xml_node_append_attr_str(n, "attr1", NULL)) == NULL);
	NDM_TEST_BREAK_IF(
		(q[1] = ndm_xml_node_append_attr_str(n, "attr2", NULL)) == NULL);
	NDM_TEST_BREAK_IF(
		(q[2] = ndm_xml_node_append_attr_str(n, "attr3", NULL)) == NULL);
	NDM_TEST_BREAK_IF(
		(q[3] = ndm_xml_node_append_attr_str(n, "attr1", NULL)) == NULL);
	NDM_TEST_BREAK_IF(
		(q[4] = ndm_xml_node_append_attr_str(n, "attr4", NULL)) == NULL);
	NDM_TEST_BREAK_IF(
		(q[5] = ndm_xml_node_append_attr_str(n, "attr1", NULL)) == NULL);

	NDM_TEST(ndm_xml_node_first_attr(n, "attr1") == q[0]);
	NDM_TEST(ndm_xml_node_last_attr(n, NULL) == q[5]);
	NDM_TEST(ndm_xml_node_last_attr(n, "attr12") == NULL);
	NDM_TEST(ndm_xml_node_last_attr(n, "attr1") == q[5]);
	NDM_TEST(
		ndm_xml_node_last_attr(n, "attr2") ==
		ndm_xml_node_first_attr(n, "attr2"));
	NDM_TEST(
		ndm_xml_node_last_attr(n, "attr3") ==
		ndm_xml_node_first_attr(n, "attr3"));
	NDM_TEST(
		ndm_xml_node_last_attr(n, "attr4") ==
		ndm_xml_node_first_attr(n, "attr4"));

	NDM_TEST(ndm_xml_attr_prev(q[0], NULL) == NULL);
	NDM_TEST(ndm_xml_attr_next(q[5], NULL) == NULL);
	NDM_TEST(ndm_xml_attr_next(q[0], NULL) == q[1]);
	NDM_TEST(ndm_xml_attr_prev(q[1], NULL) == q[0]);

	NDM_TEST(ndm_xml_attr_next(q[2], "attr1") == q[3]);
	NDM_TEST(ndm_xml_attr_next(q[2], "attr4") == q[4]);

	NDM_TEST(ndm_xml_document_copy(&copy, &d));
	NDM_TEST(ndm_xml_document_is_equal(&copy, &d));

	ndm_xml_node_remove_last_attr(n);

	NDM_TEST(!ndm_xml_document_is_equal(&copy, &d));

	ndm_xml_node_remove_first_attr(n);

	NDM_TEST(ndm_xml_attr_prev(q[1], NULL) == NULL);
	NDM_TEST(ndm_xml_attr_next(q[4], NULL) == NULL);

	ndm_xml_node_prepend_attr(n, q[0]);
	ndm_xml_node_append_attr(n, q[5]);

	NDM_TEST(ndm_xml_document_is_equal(&copy, &d));

	NDM_TEST(ndm_xml_node_first_attr(n, NULL) == q[0]);
	NDM_TEST(ndm_xml_node_last_attr(n, NULL) == q[5]);
	NDM_TEST(ndm_xml_node_first_attr(n, "attr3") == q[2]);
	NDM_TEST(ndm_xml_node_last_attr(n, "attr3") == q[2]);

	ndm_xml_node_remove_attr(n, q[2]);

	NDM_TEST(ndm_xml_node_first_attr(n, "attr3") == NULL);
	NDM_TEST(ndm_xml_node_last_attr(n, "attr3") == NULL);

	ndm_xml_node_insert_attr(n, q[1], q[2]);

	NDM_TEST(ndm_xml_node_first_attr(n, "attr3") == q[2]);
	NDM_TEST(ndm_xml_node_last_attr(n, "attr3") == q[2]);

	ndm_xml_node_remove_all_attr(n);

	NDM_TEST(ndm_xml_node_first_attr(n, NULL) == NULL);

	fp = fopen("test.xml", "r");

	NDM_TEST_BREAK_IF(fp == NULL);

	NDM_TEST(fseek(fp, 0, SEEK_END) == 0);

	fsize = ftell(fp);

	NDM_TEST(fsize > 0);

	if (fsize > 0) {
		char *text = (char *) malloc((size_t) fsize + 1);

		NDM_TEST(text != NULL);

		if (text != NULL) {
			NDM_TEST(fseek(fp, 0, SEEK_SET) == 0);
			NDM_TEST(fread(text, (size_t) fsize, 1, fp) == 1);

			text[fsize] = '\0';
			NDM_TEST(ndm_xml_document_parse(&d, text,
				NDM_XML_DOCUMENT_PARSE_FLAGS_DEFAULT) ==
					NDM_XML_DOCUMENT_PARSE_ERROR_OK);

			NDM_TEST(ndm_xml_document_copy(&copy, &d));
			NDM_TEST(ndm_xml_document_is_equal(&copy, &d));

			ndm_xml_document_clear(&copy);
		}

		free(text);
	}

	fclose(fp);
	ndm_xml_document_clear(&d);

	return NDM_TEST_RESULT;
}

