/**
 * Copyright (c) 2006, 2007 Marcin Kalicinski
 * Copyright (c) 2012 NDM Systems, Inc. http://www.ndmsystems.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ndm/int.h>
#include <ndm/xml.h>

struct ndm_xml_node_t
{
	const char *name;
	size_t name_size;
	const char *value;
	size_t value_size;
	enum ndm_xml_node_type_t type;
	struct ndm_xml_document_t *document;
	struct ndm_xml_node_t *parent;
	struct ndm_xml_node_t *first_child;
	struct ndm_xml_node_t *last_child;
	struct ndm_xml_attr_t *first_attr;
	struct ndm_xml_attr_t *last_attr;
	struct ndm_xml_node_t *next_sibling;
	struct ndm_xml_node_t *prev_sibling;
};

struct ndm_xml_attr_t
{
	const char *name;
	size_t name_size;
	const char *value;
	size_t value_size;
	struct ndm_xml_document_t *document;
	struct ndm_xml_node_t *node;
	struct ndm_xml_attr_t *next;
	struct ndm_xml_attr_t *prev;
};

static inline bool __ndm_xml_name_is_empty(const char *const name)
{
	return name == NULL || name[0] == '\0';
}

/**
 * XML parser functions.
 */

typedef bool (*predicate_t)(const char c);

static inline bool __ndm_xml_parser_whitespace_pred(const char ch)
{
	return strchr(" \n\r\t", ch) != NULL && ch != '\0';
}

static inline bool __ndm_xml_parser_node_name_pred(const char ch)
{
	return strchr(" \n\r\t/>?", ch) == NULL && ch != '\0';
}

static inline bool __ndm_xml_parser_attr_name_pred(const char ch)
{
	return strchr(" \n\r\t/<>=?!", ch) == NULL && ch != '\0';
}

static inline bool __ndm_xml_parser_text_pred(const char ch)
{
	return !((ch == '<') || (ch == '\0'));
}

static inline bool __ndm_xml_parser_text_pure_no_ws_pred(const char ch)
{
	return !((ch == '<') || (ch == '\0') || (ch == '&'));
}

static inline bool __ndm_xml_parser_text_pure_with_ws_pred(const char ch)
{
	return strchr(" \n\r\t<&", ch) == NULL && ch != '\0';
}

static inline bool __ndm_xml_parser_attr_value_quote_pred(const char ch)
{
	return !((ch == '\'') || (ch == '\0'));
}

static inline bool __ndm_xml_parser_attr_value_dquote_pred(const char ch)
{
	return !((ch == '\"') || (ch == '\0'));
}

static inline bool
__ndm_xml_parser_attr_value_pure_quote_pred(const char ch)
{
	return !((ch == '\'') || (ch == '\0') || (ch == '&'));
}

static inline bool
__ndm_xml_parser_attr_value_pure_dquote_pred(const char ch)
{
	return !((ch == '\"') || (ch == '\0') || (ch == '&'));
}

static enum ndm_xml_document_parse_error_t
__ndm_xml_parser_insert_coded_character(
		char **ptext,
		uint32_t ccode,
		const enum ndm_xml_document_parse_flags_t flags)
{
	enum ndm_xml_document_parse_error_t code =
		NDM_XML_DOCUMENT_PARSE_ERROR_OK;
	char *text = *ptext;

	if (flags & NDM_XML_DOCUMENT_PARSE_FLAGS_NO_UTF8) {
		/**
		 * Insert 8-bit ASCII character.
		 * TODO: possibly verify that code is less
		 * than 256 and use replacement char otherwise?
		 **/

		text[0] = (char) ccode;
		text += 1;
	} else {
		/**
		 * Insert UTF-8 sequence.
		 **/

		if (ccode < 0x80) {		/* 1 byte sequence */
			text[0] = (char) ccode;
			text += 1;
		} else
		if (ccode < 0x800) {	/* 2 byte sequence */
			text[1] = (char) ((ccode | 0x80) & 0xbf);
			ccode >>= 6;
			text[0] = (char) (ccode | 0xc0);
			text += 2;
		} else
		if (ccode < 0x10000) {	/* 3 byte sequence */
			text[2] = (char) ((ccode | 0x80) & 0xbf);
			ccode >>= 6;
			text[1] = (char) ((ccode | 0x80) & 0xbf);
			ccode >>= 6;
			text[0] = (char) (ccode | 0xe0);
			text += 3;
		} else
		if (ccode < 0x110000) {	/* 4 byte sequence */
			text[3] = (char) ((ccode | 0x80) & 0xbf);
			ccode >>= 6;
			text[2] = (char) ((ccode | 0x80) & 0xbf);
			ccode >>= 6;
			text[1] = (char) ((ccode | 0x80) & 0xbf);
			ccode >>= 6;
			text[0] = (char) (ccode | 0xf0);
			text += 4;
		} else {
			/**
			 * Invalid, only codes up to 0x10FFFF
			 * are allowed in Unicode.
			 **/

			code = NDM_XML_DOCUMENT_PARSE_ERROR_INVALID_CHAR_ENTITY;
		}
	}

	*ptext = text;

	return code;
}

static void __ndm_xml_parser_skip(char **ptext, predicate_t stop_pred)
{
	char *t = *ptext;

	while (stop_pred(*t)) {
		++t;
	}

	*ptext = t;
}

static enum ndm_xml_document_parse_error_t
__ndm_xml_parser_skip_and_expand_character_refs(
		char **ptext,
		predicate_t stop_pred,
		predicate_t stop_pred_pure,
		const enum ndm_xml_document_parse_flags_t flags,
		char **pend)
{
	char *text = *ptext;
	char *src = NULL;
	char *dest = NULL;
	enum ndm_xml_document_parse_error_t code =
		NDM_XML_DOCUMENT_PARSE_ERROR_OK;

	/**
	 * If entity translation, whitespace condense and whitespace
	 * trimming is disabled, use plain skip.
	 **/

	if (flags & NDM_XML_DOCUMENT_PARSE_FLAGS_NO_ENTITY_TRANSLATION &&
		!(flags & NDM_XML_DOCUMENT_PARSE_FLAGS_NORMALIZE_WHITESPACE) &&
		!(flags & NDM_XML_DOCUMENT_PARSE_FLAGS_TRIM_WHITESPACE))
	{
		__ndm_xml_parser_skip(&text, stop_pred);
		*pend = text;
		*ptext = text;

		return code;
	}

	/**
	 * Use simple skip until first modification is detected.
	 **/

	__ndm_xml_parser_skip(&text, stop_pred_pure);

	/**
	 * Use translation skip.
	 **/

	src = text;
	dest = src;

	while (stop_pred(*src)) {
		/**
		 * If entity translation is enabled.
		 **/

		if (!(flags & NDM_XML_DOCUMENT_PARSE_FLAGS_NO_ENTITY_TRANSLATION)) {
			/**
			 * Test if replacement is needed.
			 **/

			if (src[0] == '&') {
				switch (src[1]) {
					/* &amp; &apos; */

					case 'a':
						if ((src[2] == 'm') &&
							(src[3] == 'p') &&
							(src[4] == ';'))
						{
							*dest = '&';
							++dest;
							src += 5;

							continue;
						}

						if ((src[2] == 'p') &&
							(src[3] == 'o') &&
							(src[4] == 's') &&
							(src[5] == ';'))
						{
							*dest = '\'';
							++dest;
							src += 6;

							continue;
						}

						break;

					/* &quot; */

					case 'q':
						if ((src[2] == 'u') &&
							(src[3] == 'o') &&
							(src[4] == 't') &&
							(src[5] == ';'))
						{
							*dest = '"';
							++dest;
							src += 6;

							continue;
						}

						break;

					/* &gt; */

					case 'g':
						if ((src[2] == 't') &&
							(src[3] == ';'))
						{
							*dest = '>';
							++dest;
							src += 4;

							continue;
						}

						break;

					/* &lt; */

					case 'l':
						if ((src[2] == 't') &&
							(src[3] == ';'))
						{
							*dest = '<';
							++dest;
							src += 4;

							continue;
						}

						break;

					/* &#...; - assumes ASCII */

					case '#':
						if (src[2] == 'x') {
							uint32_t xcode = 0;

							src += 3;   /* Skip &#x */

							while (isxdigit(*src)) {
								xcode = xcode*16 +
									(uint32_t) (tolower(*src) - '0');
								++src;
							}

							/**
							 * Put character in output.
							 **/

							code = __ndm_xml_parser_insert_coded_character(
									&dest, xcode, flags);

							if (code != NDM_XML_DOCUMENT_PARSE_ERROR_OK) {
								*ptext = text;

								return code;
							}
						} else {
							uint32_t dcode = 0;

							src += 2;   /* Skip &# */

							while (isdigit(*src)) {
								dcode = dcode*10 + (uint32_t) (*src - '0');
								++src;
							}

							/**
							 * Put character in output.
							 **/

							code = __ndm_xml_parser_insert_coded_character(
									&dest, dcode, flags);

							if (code != NDM_XML_DOCUMENT_PARSE_ERROR_OK) {
								*ptext = text;

								return code;
							}
						}

						if (*src == ';') {
							++src;
						} else {
							*ptext = text;

							return NDM_XML_DOCUMENT_PARSE_ERROR_NO_SEMICOLON;
						}

						continue;

					/**
					 * Something else.
					 **/

					default:
						/**
						 * Ignore, just copy '&' verbatim
						 **/

						break;
				}
			}
		}

		/**
		 * If whitespace condensing is enabled.
		 **/

		if (flags & NDM_XML_DOCUMENT_PARSE_FLAGS_NORMALIZE_WHITESPACE) {
			/**
			 * Test if condensing is needed.
			 **/

			if (__ndm_xml_parser_whitespace_pred(*src)) {
				*dest = ' ';
				++dest;		/* Put single space in dest */
				++src;		/* Skip first whitespace char */

				/**
				 * Skip remaining whitespace chars.
				 **/

				while (__ndm_xml_parser_whitespace_pred(*src)) {
					++src;
				}

				continue;
			}
		}

		/**
		 * No replacement, only copy character.
		 **/

		*dest++ = *src++;
	}

	/**
	 * Return new end.
	 **/

	*ptext = src;
	*pend = dest;

	return code;
}

static enum ndm_xml_document_parse_error_t
__ndm_xml_parser_parse_node_attributes(
		struct ndm_xml_document_t *doc,
		char **ptext,
		struct ndm_xml_node_t *node,
		const enum ndm_xml_document_parse_flags_t flags)
{
	enum ndm_xml_document_parse_error_t code =
		NDM_XML_DOCUMENT_PARSE_ERROR_OK;
	char *text = *ptext;

	/**
	 * For all attributes.
	 **/

	while (__ndm_xml_parser_attr_name_pred(*text)) {
		/**
		 * Extract attribute name.
		 **/

		char *name = text;
		char *value = NULL;
		struct ndm_xml_attr_t *a = NULL;
		char *end = NULL;
		char quote = '\0';

		/**
		 * Skip first character of attribute name.
		 **/

		++text;

		__ndm_xml_parser_skip(&text, __ndm_xml_parser_attr_name_pred);

		if (text == name) {
			*ptext = text;

			return NDM_XML_DOCUMENT_PARSE_ERROR_ATTR_NAME_EXPECTED;
		}

		a = ndm_xml_document_alloc_attr(doc, NULL, NULL);

		if (a == NULL) {
			*ptext = text;

			return NDM_XML_DOCUMENT_PARSE_ERROR_OOM;
		}

		end = text;

		ndm_xml_node_append_attr(node, a);

		/**
		 * Skip whitespace after attribute name.
		 **/

		__ndm_xml_parser_skip(&text, __ndm_xml_parser_whitespace_pred);

		/**
		 * Skip =
		 **/

		if (*text != '=') {
			*ptext = text;

			return NDM_XML_DOCUMENT_PARSE_ERROR_EQUAL_SIGN_EXPECTED;
		}

		++text;
		*end = '\0';

		ndm_xml_attr_set_name(a, name);

		/**
		 * Skip whitespace after =
		 **/

		__ndm_xml_parser_skip(&text, __ndm_xml_parser_whitespace_pred);

		/**
		 * Skip quote and remember if it was ' or ".
		 **/

		quote = *text;

		if ((quote != '\'') && (quote != '"')) {
			*ptext = text;

			return NDM_XML_DOCUMENT_PARSE_ERROR_QUOTE_EXPECTED;
		}

		++text;

		/**
		 * Extract attribute value and expand char refs
		 * in it. No whitespace normalization in attributes.
		 **/

		value = text;

		code = (quote == '\'') ?
			__ndm_xml_parser_skip_and_expand_character_refs(&text,
				__ndm_xml_parser_attr_value_quote_pred,
				__ndm_xml_parser_attr_value_pure_quote_pred,
				flags &
				(enum ndm_xml_document_parse_flags_t)
					~NDM_XML_DOCUMENT_PARSE_FLAGS_NORMALIZE_WHITESPACE,
				&end) :
			__ndm_xml_parser_skip_and_expand_character_refs(&text,
				__ndm_xml_parser_attr_value_dquote_pred,
				__ndm_xml_parser_attr_value_pure_dquote_pred,
				flags &
				(enum ndm_xml_document_parse_flags_t)
					~NDM_XML_DOCUMENT_PARSE_FLAGS_NORMALIZE_WHITESPACE,
				&end);

		if (code != NDM_XML_DOCUMENT_PARSE_ERROR_OK) {
			*ptext = text;

			return code;
		}

		/**
		 * Make sure that end quote is present.
		 **/

		if (*text != quote) {
			*ptext = text;

			return NDM_XML_DOCUMENT_PARSE_ERROR_QUOTE_EXPECTED;
		}

		++text;
		*end = '\0';

		ndm_xml_attr_set_value(a, value);

		/**
		 * Skip whitespace after attribute value.
		 **/

		__ndm_xml_parser_skip(&text, __ndm_xml_parser_whitespace_pred);
	}

	*ptext = text;

	return code;
}

static enum ndm_xml_document_parse_error_t
__ndm_xml_parser_parse_xml_declaration(
		struct ndm_xml_document_t *doc,
		char **ptext,
		const enum ndm_xml_document_parse_flags_t flags,
		struct ndm_xml_node_t **decl)
{
	enum ndm_xml_document_parse_error_t code =
		NDM_XML_DOCUMENT_PARSE_ERROR_OK;
	char *text = *ptext;

	*decl = NULL;

	/**
	 * If parsing of declaration is disabled.
	 **/

	if (!(flags & NDM_XML_DOCUMENT_PARSE_FLAGS_PARSE_DECLARATION_NODE)) {
		/**
		 * Skip until end of declaration.
		 **/

		while (((text[0] != '?') || (text[1] != '>'))) {
			if (!text[0]) {
				*ptext = text;

				return NDM_XML_DOCUMENT_PARSE_ERROR_DECL_END_EXPECTED;
			}

			++text;
		}

		text += 2;	/* Skip '?>' */
	} else
	if ((*decl = ndm_xml_document_alloc_node(doc,
			NDM_XML_NODE_TYPE_DECLARATION, NULL, NULL)) == NULL)
	{
		*ptext = text;

		return NDM_XML_DOCUMENT_PARSE_ERROR_OOM;
	} else {
		/**
		 * Skip whitespace before attributes or ?>
		 **/

		__ndm_xml_parser_skip(&text, __ndm_xml_parser_whitespace_pred);

		/**
		 * Parse declaration attributes.
		 **/

		code = __ndm_xml_parser_parse_node_attributes(
			doc, &text, *decl, flags);

		/**
		 * Skip ?>
		 **/

		if (code == NDM_XML_DOCUMENT_PARSE_ERROR_OK &&
			(text[0] != '?' || text[1] != '>'))
		{
			*ptext = text;

			return NDM_XML_DOCUMENT_PARSE_ERROR_DECL_END_EXPECTED;
		}

		text += 2;
	}

	*ptext = text;

	return code;
}

static enum ndm_xml_document_parse_error_t
__ndm_xml_parser_parse_comment(
		struct ndm_xml_document_t *doc,
		char **ptext,
		const enum ndm_xml_document_parse_flags_t flags,
		struct ndm_xml_node_t **comment)
{
	enum ndm_xml_document_parse_error_t code =
		NDM_XML_DOCUMENT_PARSE_ERROR_OK;
	char *text = *ptext;
	char *value = text;

	*comment = NULL;

	/**
	 * Skip until end of comment.
	 **/

	while ((text[0] != '-') || (text[1] != '-') || (text[2] != '>')) {
		if( !text[0] ) {
			*ptext = text;

			return NDM_XML_DOCUMENT_PARSE_ERROR_UNEXPECTED_STREAM_END;
		}

		++text;
	}

	/**
	 * If parsing of comments is enabled.
	 **/

	if (flags & NDM_XML_DOCUMENT_PARSE_FLAGS_PARSE_COMMENT_NODES) {
		if ((*comment = ndm_xml_document_alloc_node(
				doc, NDM_XML_NODE_TYPE_COMMENT, NULL, NULL)) == NULL)
		{
			*ptext = text;

			return NDM_XML_DOCUMENT_PARSE_ERROR_OOM;
		}

		*text = '\0';
		ndm_xml_node_set_value(*comment, value);
	}

	text += 3;	/* Skip '-->' */
	*ptext = text;

	return code;
}

static enum ndm_xml_document_parse_error_t
__ndm_xml_parser_parse_doctype(
		struct ndm_xml_document_t *doc,
		char **ptext,
		const enum ndm_xml_document_parse_flags_t flags,
		struct ndm_xml_node_t **doctype)
{
	enum ndm_xml_document_parse_error_t code =
		NDM_XML_DOCUMENT_PARSE_ERROR_OK;
	char *text = *ptext;
	char *value = text;

	*doctype = NULL;

	/**
	 * Skip to >
	 **/

	while (*text != '>') {
		const char t = *text;

		if (t == '[') {
			int depth = 1;

			/**
			 * If '[' encountered, scan for matching ending ']'
			 * using naive algorithm with depth.
			 * This works for all W3C test files
			 * except for 2 most wicked.
			 **/

			++text;		/* Skip '[' */

			while (depth > 0) {
				const char c = *text;

				if (c == '[') {
					++depth;
				} else
				if (c == ']') {
					--depth;
				} else
				if (c == '\0') {
					*ptext = text;

					return
						NDM_XML_DOCUMENT_PARSE_ERROR_UNEXPECTED_STREAM_END;
				} else {
					++text;
				}
			}
		} else
		if (t == '\0') {
			*ptext = text;

			return NDM_XML_DOCUMENT_PARSE_ERROR_UNEXPECTED_STREAM_END;
		} else {
			++text;
		}
	}

	/**
	 * If DOCTYPE nodes enabled.
	 **/

	if (flags & NDM_XML_DOCUMENT_PARSE_FLAGS_PARSE_DOCTYPE_NODE) {
		if ((*doctype = ndm_xml_document_alloc_node(
				doc, NDM_XML_NODE_TYPE_DOCTYPE, NULL, NULL)) == NULL)
		{
			*ptext = text;

			return NDM_XML_DOCUMENT_PARSE_ERROR_OOM;
		}

		*text = '\0';
		ndm_xml_node_set_value(*doctype, value);
	}

	++text;			/* skip '>' */
	*ptext = text;

	return code;
}

static enum ndm_xml_document_parse_error_t
__ndm_xml_parser_parse_pi(
		struct ndm_xml_document_t *doc,
		char **ptext,
		const enum ndm_xml_document_parse_flags_t flags,
		struct ndm_xml_node_t **pi)
{
	enum ndm_xml_document_parse_error_t code =
		NDM_XML_DOCUMENT_PARSE_ERROR_OK;
	char *text = *ptext;

	*pi = NULL;

	/**
	 * If creation of PI nodes is enabled.
	 **/

	if (flags & NDM_XML_DOCUMENT_PARSE_FLAGS_PARSE_PI_NODES) {
		/**
		 * Create pi node.
		 **/
		char *name = NULL;
		char *end = NULL;
		char *value = NULL;

		if ((*pi = ndm_xml_document_alloc_node(
				doc, NDM_XML_NODE_TYPE_PI, NULL, NULL)) == NULL)
		{
			*ptext = text;

			return NDM_XML_DOCUMENT_PARSE_ERROR_OOM;
		}

		/**
		 * Extract PI target name.
		 **/

		name = text;

		__ndm_xml_parser_skip(&text, __ndm_xml_parser_node_name_pred);

		if (text == name) {
			*ptext = text;

			return NDM_XML_DOCUMENT_PARSE_ERROR_PI_TARGET_EXPECTED;
		}

		end = text;

		/**
		 * Skip whitespace between pi target and pi.
		 **/

		__ndm_xml_parser_skip(&text, __ndm_xml_parser_whitespace_pred);

		/**
		 * Remember start of pi.
		 **/

		value = text;

		/**
		 * Skip to '?>'
		 **/

		while ((text[0] != '?') || (text[1] != '>')) {
			if (*text == '\0') {
				*ptext = text;

				return NDM_XML_DOCUMENT_PARSE_ERROR_DECL_END_EXPECTED;
			}

			++text;
		}

		/**
		 * Set pi value (verbatim, no entity expansion
		 * or whitespace normalization).
		 **/

		*end = '\0';
		*text = '\0';

		ndm_xml_node_set_name(*pi, name);
		ndm_xml_node_set_value(*pi, value);

		/**
		 * Skip '?>'
		 **/

		text += 2;
	} else {
		/**
		 * Skip to '?>'
		 **/

		while ((text[0] != '?') || (text[1] != '>')) {
			if (*text == '\0') {
				*ptext = text;

				return NDM_XML_DOCUMENT_PARSE_ERROR_DECL_END_EXPECTED;
			}

			++text;
		}

		/**
		 * Skip '?>'
		 **/

		text += 2;
	}

	*ptext = text;

	return code;
}

static enum ndm_xml_document_parse_error_t
__ndm_xml_parser_parse_cdata(
		struct ndm_xml_document_t *doc,
		char **ptext,
		const enum ndm_xml_document_parse_flags_t flags,
		struct ndm_xml_node_t **cdata)
{
	enum ndm_xml_document_parse_error_t code =
		NDM_XML_DOCUMENT_PARSE_ERROR_OK;
	char *text = *ptext;
	char *value = text;

	*cdata = NULL;

	/**
	 * Skip until end of cdata.
	 **/

	while ((text[0] != ']') || (text[1] != ']') || (text[2] != '>')) {
		if (!text[0]) {
			*ptext = text;

			return NDM_XML_DOCUMENT_PARSE_ERROR_UNEXPECTED_CDATA_END;
		}

		++text;
	}

	/**
	 * If CDATA is enabled.
	 **/

	if (!(flags & NDM_XML_DOCUMENT_PARSE_FLAGS_NO_DATA_NODES)) {
		if ((*cdata = ndm_xml_document_alloc_node(
				doc, NDM_XML_NODE_TYPE_CDATA, NULL, NULL)) == NULL)
		{
			*ptext = text;

			return NDM_XML_DOCUMENT_PARSE_ERROR_OOM;
		}

		*text = '\0';
		ndm_xml_node_set_value(*cdata, value);

		/**
		 * Skip ]]>
		 **/

		text += 3;
	}

	*ptext = text;

	return code;
}

static enum ndm_xml_document_parse_error_t
__ndm_xml_parser_parse_node_contents(
		struct ndm_xml_document_t *doc,
		char **ptext,
		struct ndm_xml_node_t *node,
		const enum ndm_xml_document_parse_flags_t flags);

static enum ndm_xml_document_parse_error_t
__ndm_xml_parser_parse_element(
		struct ndm_xml_document_t *doc,
		char **ptext,
		const enum ndm_xml_document_parse_flags_t flags,
		struct ndm_xml_node_t **element)
{
	enum ndm_xml_document_parse_error_t code =
		NDM_XML_DOCUMENT_PARSE_ERROR_OK;
	char *text = *ptext;
	char *name = NULL;
	char *end = NULL;

	if ((*element = ndm_xml_document_alloc_node(doc,
			NDM_XML_NODE_TYPE_ELEMENT, NULL, NULL)) == NULL)
	{
		return NDM_XML_DOCUMENT_PARSE_ERROR_OOM;
	}

	name = text;

	__ndm_xml_parser_skip(&text, __ndm_xml_parser_node_name_pred);

	if (text == name) {
		*ptext = text;

		return NDM_XML_DOCUMENT_PARSE_ERROR_NODE_NAME_EXPECTED;
	}

	end = text;

	/**
	 * Skip whitespace between element name and attributes or >
	 **/

	__ndm_xml_parser_skip(&text, __ndm_xml_parser_whitespace_pred);

	ndm_xml_node_set_name(*element, name);

	/**
	 * Parse attributes, if any.
	 **/

	code = __ndm_xml_parser_parse_node_attributes(
		doc, &text, *element, flags);

	if (code != NDM_XML_DOCUMENT_PARSE_ERROR_OK) {
		*ptext = text;

		return code;
	}

	/**
	 * Determine ending type.
	 **/

	if (*text == '>') {
		++text;

		*end = '\0';
		ndm_xml_node_set_name(*element, name);

		code = __ndm_xml_parser_parse_node_contents(
			doc, &text, *element, flags);

		if (code != NDM_XML_DOCUMENT_PARSE_ERROR_OK) {
			*ptext = text;

			return code;
		}
	} else
	if (*text == '/') {
		++text;

		*end = '\0';
		ndm_xml_node_set_name(*element, name);

		if (*text != '>') {
			*ptext = text;

			return NDM_XML_DOCUMENT_PARSE_ERROR_RBRACKET_EXPECTED;
		}

		++text;
	} else {
		*ptext = text;

		return NDM_XML_DOCUMENT_PARSE_ERROR_RBRACKET_EXPECTED;
	}

	*end = '\0';
	*ptext = text;

	return code;
}

static enum ndm_xml_document_parse_error_t
__ndm_xml_parser_parse_node(
		struct ndm_xml_document_t *doc,
		char **ptext,
		const enum ndm_xml_document_parse_flags_t flags,
		struct ndm_xml_node_t **node)
{
	enum ndm_xml_document_parse_error_t code =
		NDM_XML_DOCUMENT_PARSE_ERROR_OK;
	char *text = *ptext;

	/**
	 * Parse proper node type.
	 **/

	switch (text[0]) {
		/**
		 * <...
		 **/
		default: {
			/**
			 * Parse and append element node
			 **/

			code = __ndm_xml_parser_parse_element(doc, &text, flags, node);
			*ptext = text;

			return code;
		}

		/**
		 * <?...
		 */

		case '?': {
			/**
			 * Skip ?
			 **/

			++text;

			if (((text[0] == 'x') || (text[0] == 'X')) &&
				((text[1] == 'm') || (text[1] == 'M')) &&
				((text[2] == 'l') || (text[2] == 'L')) &&
				__ndm_xml_parser_whitespace_pred(text[3]))
			{
				/**
				 * '<?xml ' - skip xml declaration.
				 **/

				text += 4;

				code = __ndm_xml_parser_parse_xml_declaration(
					doc, &text, flags, node);
				*ptext = text;

				return code;
			} else {
				/**
				 * Parse PI.
				 **/

				code = __ndm_xml_parser_parse_pi(doc, &text, flags, node);
				*ptext = text;

				return code;
			}
		}

		/**
		 * <!...
		 **/

		case '!': {
			/**
			 * Parse proper subset of <! node.
			 **/

			switch (text[1]) {
				/**
				 * <!-
				 **/

				case '-': {
					if (text[2] == '-') {
						/**
						 * '<!--' - skip xml comment.
						 **/

						text += 3;

						code = __ndm_xml_parser_parse_comment(
							doc, &text, flags, node);
						*ptext = text;

						return code;
					}

					break;
				}

				/**
				 * <![
				 **/

				case '[': {
					if ((text[2] == 'C') &&
						(text[3] == 'D') &&
						(text[4] == 'A') &&
						(text[5] == 'T') &&
						(text[6] == 'A') &&
						(text[7] == '['))
					{
						/**
						 * '<![CDATA[' - skip cdata.
						 **/

						text += 8;

						code = __ndm_xml_parser_parse_cdata(
							doc, &text, flags, node);
						*ptext = text;

						return code;
					}

					break;
				}

				/**
				 * <!D
				 **/

				case 'D': {
					if ((text[2] == 'O') &&
						(text[3] == 'C') &&
						(text[4] == 'T') &&
						(text[5] == 'Y') &&
						(text[6] == 'P') &&
						(text[7] == 'E') &&
						__ndm_xml_parser_whitespace_pred(text[8]))
					{
						/**
						 * '<!DOCTYPE ' - skip doctype.
						 **/

						text += 9;

						code = __ndm_xml_parser_parse_doctype(
							doc, &text, flags, node);
						*ptext = text;

						return code;
					}
				}
			}	/* switch */

			/**
			 * Attempt to skip other, unrecognized node types
			 * starting with <!
			 **/

			++text;		/* Skip ! */

			while (*text != '>') {
				if (*text == '\0') {
					*ptext = text;

					return NDM_XML_DOCUMENT_PARSE_ERROR_RBRACKET_EXPECTED;
				}

				++text;
			}

			/**
			 * Skip '>'
			 **/

			++text;

			/**
			 * No node recognized.
			 **/

			*node = NULL;
			*ptext = text;

			return code;
		}
	}
}

static enum ndm_xml_document_parse_error_t
__ndm_xml_parser_parse_and_append_data(
		struct ndm_xml_document_t *doc,
		struct ndm_xml_node_t *node,
		char **ptext,
		char *contents_start,
		const enum ndm_xml_document_parse_flags_t flags,
		char *next)
{
	char *text = *ptext;
	char *value = NULL;
	char *end = NULL;
	enum ndm_xml_document_parse_error_t code =
		NDM_XML_DOCUMENT_PARSE_ERROR_OK;

	/**
	 * Backup to contents start if whitespace trimming is disabled.
	 **/

	if (!(flags & NDM_XML_DOCUMENT_PARSE_FLAGS_TRIM_WHITESPACE)) {
		text = contents_start;
	}

	/**
	 * Skip until end of data.
	 **/

	value = text;
	code =
		(flags & NDM_XML_DOCUMENT_PARSE_FLAGS_NORMALIZE_WHITESPACE) ?
		__ndm_xml_parser_skip_and_expand_character_refs(
			&text, __ndm_xml_parser_text_pred,
			__ndm_xml_parser_text_pure_with_ws_pred, flags, &end) :
		__ndm_xml_parser_skip_and_expand_character_refs(
			&text, __ndm_xml_parser_text_pred,
			__ndm_xml_parser_text_pure_no_ws_pred, flags, &end);

	if (code != NDM_XML_DOCUMENT_PARSE_ERROR_OK) {
		*ptext = text;

		return code;
	}

	/**
	 * Trim trailing whitespace if flag is set;
	 * leading was already trimmed by whitespace skip after >
	 **/

	if (flags & NDM_XML_DOCUMENT_PARSE_FLAGS_TRIM_WHITESPACE) {
		if (flags & NDM_XML_DOCUMENT_PARSE_FLAGS_NORMALIZE_WHITESPACE) {
			/**
			 * Whitespace is already condensed to single space
			 * characters by skipping function,
			 * so just trim 1 char off the end.
			 **/

			if (end > text && *(end - 1) == ' ') {
				--end;
			}
		} else {
			/**
			 * Backup until non-whitespace character is found.
			 **/

			while (
				end > text &&
				__ndm_xml_parser_whitespace_pred(*(end - 1)))
			{
				--end;
			}
		}
	}

	/**
	 * If characters are still left between @c end and @c value
	 * (this test is only necessary if normalization is enabled).
	 * Create new data node.
	 **/

	if (!(flags & NDM_XML_DOCUMENT_PARSE_FLAGS_NO_DATA_NODES)) {
		struct ndm_xml_node_t *data =
			ndm_xml_document_alloc_node(
				doc, NDM_XML_NODE_TYPE_DATA, NULL, NULL);

		if (data == NULL) {
			*ptext = text;

			return NDM_XML_DOCUMENT_PARSE_ERROR_OOM;
		}

		ndm_xml_node_append_child(node, data);
	}

	/**
	 * Return character that ends data;
	 * this is required because zero terminator overwritten it.
	 * Place zero terminator after value.
	 **/

	*next = *text;
	*end = '\0';

	*ptext = text;

	/**
	 * Add data to parent node if no data exists yet.
	 **/

	if (!(flags & NDM_XML_DOCUMENT_PARSE_FLAGS_NO_ELEMENT_VALUES) &&
		__ndm_xml_name_is_empty(ndm_xml_node_name(node)))
	{
		ndm_xml_node_set_value(node, value);
	}

	return code;
}

static enum ndm_xml_document_parse_error_t
__ndm_xml_parser_parse_node_contents(
		struct ndm_xml_document_t *doc,
		char **ptext,
		struct ndm_xml_node_t *node,
		const enum ndm_xml_document_parse_flags_t flags)
{
	enum ndm_xml_document_parse_error_t code =
		NDM_XML_DOCUMENT_PARSE_ERROR_OK;
	char *text = *ptext;

	/**
	 * For all children and text.
	 **/

	do {
		/**
		 * Skip whitespace between > and node contents.
		 * Store start of node contents
		 * before whitespace is skipped.
		 **/

		char *contents_start = text;
		char next = '\0';

		__ndm_xml_parser_skip(&text, __ndm_xml_parser_whitespace_pred);

		next = *text;

		/**
		 * After data nodes, instead of continuing the loop,
		 * control jumps here. This is because zero termination
		 * inside ParseAndAppendData() function would wreak
		 * havoc with the above code. Also, skipping whitespace
		 * after data nodes is unnecessary.
		 **/

after_data_node:

		/**
		 * Determine what comes next: node closing,
		 * child node, data node, or 0?
		 **/

		if (next == '<') {
			/**
			 * Node closing or child node.
			 **/

			if (text[1] == '/') {
				/**
				 * Node closing. Skip '</'
				 **/

				text += 2;

				if (flags & NDM_XML_DOCUMENT_PARSE_FLAGS_CHECK_CLOSING_TAGS) {
					/**
					 * Skip and validate closing tag name.
					 **/

					char *closing_name = text;

					__ndm_xml_parser_skip(&text,
						__ndm_xml_parser_node_name_pred);

					if (strncmp(
							ndm_xml_node_name(node), closing_name,
							(size_t) (text - closing_name)) != 0)
					{
						*ptext = text;

						return NDM_XML_DOCUMENT_PARSE_ERROR_CLOSING_TAG;
					}
				} else {
					/**
					 * No validation, just skip name.
					 **/

					__ndm_xml_parser_skip(&text,
						__ndm_xml_parser_node_name_pred);
				}

				/**
				 * Skip remaining whitespace after node name.
				 **/

				__ndm_xml_parser_skip(&text,
					__ndm_xml_parser_whitespace_pred);

				if (*text != '>') {
					*ptext = text;

					return NDM_XML_DOCUMENT_PARSE_ERROR_RBRACKET_EXPECTED;
				}

				/**
				 * Skip '>'
				 **/

				++text;

				/**
				 * Node closed, finished parsing contents.
				 **/

				*ptext = text;

				return code;
			} else {
				/**
				 * Child node, skip '<'.
				 **/

				struct ndm_xml_node_t *child;

				++text;

				code = __ndm_xml_parser_parse_node(
					doc, &text, flags, &child);

				if (code != NDM_XML_DOCUMENT_PARSE_ERROR_OK) {
					/**
					 * A child node was not properly parsed.
					 **/

					*ptext = text;

					return code;
				}

				ndm_xml_node_append_child(node, child);
			}
		} else
		if (next == '\0') {
			/**
			 * End of data - error.
			 **/

			*ptext = text;

			return NDM_XML_DOCUMENT_PARSE_ERROR_UNEXPECTED_STREAM_END;
		} else {
			/**
			 * Data node.
			 **/

			code = __ndm_xml_parser_parse_and_append_data(
					doc, node, &text, contents_start, flags, &next);

			if (code != NDM_XML_DOCUMENT_PARSE_ERROR_OK) {
				*ptext = text;

				return code;
			}

			/**
			 * Bypass regular processing after data nodes.
			 **/

			goto after_data_node;
		}
	} while (true);
}

static enum ndm_xml_document_parse_error_t __ndm_xml_parser_do(
		struct ndm_xml_document_t *doc,
		char *text,
		const enum ndm_xml_document_parse_flags_t flags)
{
	enum ndm_xml_document_parse_error_t code =
		NDM_XML_DOCUMENT_PARSE_ERROR_OK;
	struct ndm_xml_node_t *root = NULL;

	assert (text != NULL);

	/**
	 * Remove current contents.
	 **/

	ndm_xml_document_clear(doc);

	if ((root = ndm_xml_document_alloc_root(doc)) == NULL) {
		return NDM_XML_DOCUMENT_PARSE_ERROR_OOM;
	}

	if (!(flags & NDM_XML_DOCUMENT_PARSE_FLAGS_NO_UTF8)) {
		/**
		 * Parse BOM, if any.
		 **/

		if (((unsigned char) (text[0]) == 0xef) &&
			((unsigned char) (text[1]) == 0xbb) &&
			((unsigned char) (text[2]) == 0xbf))
		{
			text += 3;      /* Skip UTF-8 BOM */
		}
	}

	/**
	 * Parse children.
	 **/

	do {
		/**
		 * Skip whitespace before node.
		 **/

		__ndm_xml_parser_skip(&text, __ndm_xml_parser_whitespace_pred);

		if (*text != '\0') {
			/**
			 * Parse and append a new child.
			 **/

			if (*text == '<') {
				struct ndm_xml_node_t *node;

				++text;		/* Skip '<'. */

				code = __ndm_xml_parser_parse_node(doc, &text, flags, &node);

				if (code != NDM_XML_DOCUMENT_PARSE_ERROR_OK) {
					return code;
				}

				/**
				 * A node can be parsed but not recognized.
				 **/

				if (node != NULL) {
					ndm_xml_node_append_child(root, node);
				}
			} else {
				return NDM_XML_DOCUMENT_PARSE_ERROR_LBRACKET_EXPECTED;
			}
		}
	} while (*text != '\0');

	return code;
}

/**
 * XML document functions.
 **/

void ndm_xml_document_init(
		struct ndm_xml_document_t *doc,
		void *static_buffer,
		const size_t static_buffer_size,
		const size_t dynamic_buffer_size)
{
	doc->__root = NULL;
	ndm_pool_init(&doc->__pool, static_buffer,
		static_buffer_size, dynamic_buffer_size);
}

static struct ndm_xml_node_t *__ndm_xml_document_copy_node(
		const struct ndm_xml_node_t *node,
		struct ndm_xml_document_t *dest)
{
	struct ndm_xml_node_t *new_node =
		ndm_xml_document_alloc_node(dest, ndm_xml_node_type(node),
			ndm_xml_document_alloc_str(dest, ndm_xml_node_name(node)),
			ndm_xml_document_alloc_str(dest, ndm_xml_node_value(node)));

	if (new_node != NULL) {
		struct ndm_xml_attr_t *attr = ndm_xml_node_first_attr(node, NULL);
		struct ndm_xml_node_t *child = ndm_xml_node_first_child(node, NULL);

		while (
			attr != NULL &&
			ndm_xml_node_append_attr_str(
				new_node,
				ndm_xml_attr_name(attr),
				ndm_xml_attr_value(attr)) != NULL)
		{
			attr = ndm_xml_attr_next(attr, NULL);
		}

		while (child != NULL && ndm_xml_document_is_valid(dest)) {
			struct ndm_xml_node_t *new_child =
				__ndm_xml_document_copy_node(child, dest);

			if (new_child != NULL) {
				ndm_xml_node_append_child(new_node, new_child);
				child = ndm_xml_node_next_sibling(child, NULL);
			}
		}
	}

	return ndm_xml_document_is_valid(dest) ? new_node : NULL;
}

bool ndm_xml_document_copy(
		struct ndm_xml_document_t *dest,
		const struct ndm_xml_document_t *source)
{
	bool copied = true;

	ndm_xml_document_clear(dest);

	if (!ndm_xml_document_is_empty(source)) {
		dest->__root = __ndm_xml_document_copy_node(source->__root, dest);

		if (!ndm_xml_document_is_valid(dest)) {
			ndm_xml_document_clear(dest);
			copied = false;
		}
	}

	return copied;
}

static bool __ndm_xml_node_is_equal(
		const struct ndm_xml_node_t *node,
		const struct ndm_xml_node_t *other)
{
	bool equal =
		ndm_xml_node_type(node) == ndm_xml_node_type(other) &&
		ndm_xml_node_name_size(node) == ndm_xml_node_name_size(other) &&
		ndm_xml_node_value_size(node) == ndm_xml_node_value_size(other) &&
		strcmp(ndm_xml_node_name(node), ndm_xml_node_name(other)) == 0 &&
		strcmp(ndm_xml_node_value(node), ndm_xml_node_value(other)) == 0;

	if (equal) {
		struct ndm_xml_attr_t *attr =
			ndm_xml_node_first_attr(node, NULL);
		struct ndm_xml_attr_t *other_attr =
			ndm_xml_node_first_attr(other, NULL);

		while (attr != NULL && other_attr != NULL && equal) {
			equal =
				ndm_xml_attr_name_size(attr) ==
					ndm_xml_attr_name_size(other_attr) &&
				ndm_xml_attr_value_size(attr) ==
					ndm_xml_attr_value_size(other_attr) &&
				(strcmp(
					ndm_xml_attr_name(attr),
					ndm_xml_attr_name(other_attr)) == 0) &&
				(strcmp(
					ndm_xml_attr_value(attr),
					ndm_xml_attr_value(other_attr)) == 0);
			attr = ndm_xml_attr_next(attr, NULL);
			other_attr = ndm_xml_attr_next(other_attr, NULL);
		}

		equal = equal && attr == NULL && other_attr == NULL;

		if (equal) {
			struct ndm_xml_node_t *child =
				ndm_xml_node_first_child(node, NULL);
			struct ndm_xml_node_t *other_child =
				ndm_xml_node_first_child(other, NULL);

			while (child != NULL && other_child != NULL && equal) {
				equal = __ndm_xml_node_is_equal(child, other_child);
				child = ndm_xml_node_next_sibling(child, NULL);
				other_child = ndm_xml_node_next_sibling(other_child, NULL);
			}

			equal = equal && child == NULL && other_child == NULL;
		}
	}

	return equal;
}

bool ndm_xml_document_is_equal(
		const struct ndm_xml_document_t *doc,
		const struct ndm_xml_document_t *other)
{
	bool equal = false;

	if (ndm_xml_document_is_valid(doc) &&
		ndm_xml_document_is_valid(other))
	{
		if (ndm_xml_document_is_empty(doc) &&
			ndm_xml_document_is_empty(other))
		{
			equal = true;
		} else
		if (!ndm_xml_document_is_empty(doc) &&
			!ndm_xml_document_is_empty(other))
		{
			equal = __ndm_xml_node_is_equal(doc->__root, other->__root);
		} else {
			/* not equal */
		}
	}

	return equal;
}

enum ndm_xml_document_parse_error_t ndm_xml_document_parse(
		struct ndm_xml_document_t *doc,
		char *text,
		const enum ndm_xml_document_parse_flags_t flags)
{
	enum ndm_xml_document_parse_error_t code =
		__ndm_xml_parser_do(doc, text, flags);

	if (code != NDM_XML_DOCUMENT_PARSE_ERROR_OK ||
		!ndm_xml_document_is_valid(doc))
	{
		ndm_xml_document_clear(doc);

		if (!ndm_xml_document_is_valid(doc)) {
			code = NDM_XML_DOCUMENT_PARSE_ERROR_OOM;
		}
	}

	return code;
}

void *ndm_xml_document_alloc(
		struct ndm_xml_document_t *doc,
		const size_t size)
{
	return ndm_pool_malloc(&doc->__pool, size);
}

struct ndm_xml_node_t *ndm_xml_document_alloc_root(
		struct ndm_xml_document_t *doc)
{
	ndm_xml_document_clear(doc);

	if ((doc->__root = ndm_xml_document_alloc_node(
			doc, NDM_XML_NODE_TYPE_DOCUMENT, NULL, NULL)) == NULL)
	{
		ndm_xml_document_clear(doc);
	}

	return ndm_xml_document_root(doc);
}

struct ndm_xml_node_t *ndm_xml_document_alloc_node(
		struct ndm_xml_document_t *doc,
		const enum ndm_xml_node_type_t type,
		const char *const name,
		const char *const value)
{
	struct ndm_xml_node_t *node = (struct ndm_xml_node_t *)
		ndm_pool_malloc(&doc->__pool, sizeof(*node));

	if (node != NULL) {
		ndm_xml_node_set_name(node, name);
		ndm_xml_node_set_value(node, value);
		node->type = type;
		node->document = doc;
		node->parent = NULL;
		node->first_child = NULL;
		node->last_child = NULL;
		node->first_attr = NULL;
		node->last_attr = NULL;
		node->next_sibling = NULL;
		node->prev_sibling = NULL;
	}

	return node;
}

struct ndm_xml_attr_t *ndm_xml_document_alloc_attr(
		struct ndm_xml_document_t *doc,
		const char *const name,
		const char *const value)
{
	struct ndm_xml_attr_t *attr = (struct ndm_xml_attr_t *)
		ndm_pool_malloc(&doc->__pool, sizeof(*attr));

	if (attr != NULL) {
		ndm_xml_attr_set_name(attr, name);
		ndm_xml_attr_set_value(attr, value);
		attr->document = doc;
		attr->node = NULL;
		attr->next = NULL;
		attr->prev = NULL;
	}

	return attr;
}

char *ndm_xml_document_alloc_str(
		struct ndm_xml_document_t *doc,
		const char *const s)
{
	return ndm_pool_strdup(&doc->__pool, s);
}

char *ndm_xml_document_alloc_strn(
		struct ndm_xml_document_t *doc,
		const char *const s,
		const size_t size)
{
	return ndm_pool_strndup(&doc->__pool, s, size);
}

void ndm_xml_document_clear(
		struct ndm_xml_document_t *doc)
{
	ndm_pool_clear(&doc->__pool);
	doc->__root = NULL;
}

bool ndm_xml_document_is_valid(
		const struct ndm_xml_document_t *doc)
{
	return ndm_pool_is_valid(&doc->__pool);
}

bool ndm_xml_document_is_empty(
		const struct ndm_xml_document_t *doc)
{
	return doc->__root == NULL;
}

size_t ndm_xml_document_size(
		const struct ndm_xml_document_t *doc)
{
	return ndm_pool_total_dynamic_size(&doc->__pool);
}

size_t ndm_xml_document_allocated_size(
		const struct ndm_xml_document_t *doc)
{
	return ndm_pool_allocated(&doc->__pool);
}

struct ndm_xml_node_t *ndm_xml_document_root(
		const struct ndm_xml_document_t *doc)
{
	return doc->__root;
}

/**
 * XML node functions.
 */

enum ndm_xml_node_type_t ndm_xml_node_type(
		const struct ndm_xml_node_t *node)
{
	return node->type;
}

const char *ndm_xml_node_name(
		const struct ndm_xml_node_t *node)
{
	return node->name;
}

size_t ndm_xml_node_name_size(
		const struct ndm_xml_node_t *node)
{
	return node->name_size;
}

const char *ndm_xml_node_value(
		const struct ndm_xml_node_t *node)
{
	return node->value;
}

size_t ndm_xml_node_value_size(
		const struct ndm_xml_node_t *node)
{
	return node->value_size;
}

struct ndm_xml_node_t *ndm_xml_node_parent(
		const struct ndm_xml_node_t *node)
{
	return node->parent;
}

void ndm_xml_node_set_name(
		struct ndm_xml_node_t *node,
		const char *const name)
{
	node->name = (name == NULL) ? "" : name;
	node->name_size = strlen(node->name);
}

void ndm_xml_node_set_value(
		struct ndm_xml_node_t *node,
		const char *const value)
{
	node->value = (value == NULL) ? "" : value;
	node->value_size = strlen(node->value);
}

struct ndm_xml_document_t *ndm_xml_node_document(
		const struct ndm_xml_node_t *node)
{
	return node->document;
}

struct ndm_xml_node_t *ndm_xml_node_first_child(
		const struct ndm_xml_node_t *node,
		const char *const name)
{
	struct ndm_xml_node_t *n = node->first_child;

	if (!__ndm_xml_name_is_empty(name)) {
		while (n != NULL && strcmp(n->name, name) != 0) {
			n = n->next_sibling;
		}
	}

	return n;
}

struct ndm_xml_node_t *ndm_xml_node_last_child(
		const struct ndm_xml_node_t *node,
		const char *const name)
{
	struct ndm_xml_node_t *n = node->last_child;

	if (node->first_child != NULL &&
		!__ndm_xml_name_is_empty(name))
	{
		while (n != NULL && strcmp(n->name, name) != 0) {
			n = n->prev_sibling;
		}
	}

	return n;
}

struct ndm_xml_node_t *ndm_xml_node_next_sibling(
		const struct ndm_xml_node_t *node,
		const char *const name)
{
	struct ndm_xml_node_t* s = node->next_sibling;

	/* cannot query for siblings if a node has no parent */
	assert (node->parent != NULL);

	if (!__ndm_xml_name_is_empty(name)) {
		while (s != NULL && strcmp(s->name, name) != 0) {
			s = s->next_sibling;
		}
	}

	return s;
}

struct ndm_xml_node_t *ndm_xml_node_prev_sibling(
		const struct ndm_xml_node_t *node,
		const char *const name)
{
	struct ndm_xml_node_t* s = node->prev_sibling;

	/* cannot query for siblings if a node has no parent */
	assert (node->parent != NULL);

	if (!__ndm_xml_name_is_empty(name)) {
		while (s != NULL && strcmp(s->name, name) != 0) {
			s = s->prev_sibling;
		}
	}

	return s;
}

struct ndm_xml_attr_t *ndm_xml_node_first_attr(
		const struct ndm_xml_node_t *node,
		const char *const name)
{
	struct ndm_xml_attr_t* a = node->first_attr;

	if (!__ndm_xml_name_is_empty(name)) {
		while (a != NULL && strcmp(a->name, name) != 0) {
			a = a->next;
		}
	}

	return a;
}

struct ndm_xml_attr_t *ndm_xml_node_last_attr(
		const struct ndm_xml_node_t *node,
		const char *const name)
{
	struct ndm_xml_attr_t *a = node->last_attr;

	if (node->first_attr != NULL &&
		!__ndm_xml_name_is_empty(name))
	{
		while (a != NULL && strcmp(a->name, name) != 0) {
			a = a->prev;
		}
	}

	return a;
}

void ndm_xml_node_prepend_child(
		struct ndm_xml_node_t *node,
		struct ndm_xml_node_t *child)
{
	assert(
		child != NULL &&
		child->parent == NULL &&
		child->type != NDM_XML_NODE_TYPE_DOCUMENT);
	assert (node->document == child->document);

	if (node->first_child != NULL) {
		child->next_sibling = node->first_child;
		node->first_child->prev_sibling = child;
	} else {
		child->next_sibling = NULL;
		node->last_child = child;
	}

	node->first_child = child;
	child->parent = node;
	child->prev_sibling = NULL;
}

void ndm_xml_node_append_child(
		struct ndm_xml_node_t *node,
		struct ndm_xml_node_t *child)
{
	assert (
		child != NULL &&
		child->parent == NULL &&
		child->type != NDM_XML_NODE_TYPE_DOCUMENT);
	assert (node->document == child->document);

	if (node->first_child != NULL) {
		child->prev_sibling = node->last_child;
		node->last_child->next_sibling = child;
	} else {
		child->prev_sibling = NULL;
		node->first_child = child;
	}

	node->last_child = child;
	child->parent = node;
	child->next_sibling = NULL;
}

struct ndm_xml_node_t *ndm_xml_node_append_child_str(
		struct ndm_xml_node_t *node,
		const char *const name,
		const char *const value)
{
	struct ndm_xml_document_t *doc = ndm_xml_node_document(node);
	struct ndm_xml_node_t *new_node =
		ndm_xml_document_alloc_node(doc, NDM_XML_NODE_TYPE_ELEMENT,
			name == NULL ? "" : ndm_xml_document_alloc_str(doc, name),
			value == NULL ? "" : ndm_xml_document_alloc_str(doc, value));

	if (ndm_xml_document_is_valid(doc)) {
		ndm_xml_node_append_child(node, new_node);

		return new_node;
	}

	return NULL;
}

struct ndm_xml_node_t *ndm_xml_node_append_child_int(
		struct ndm_xml_node_t *node,
		const char *const name,
		const intmax_t value)
{
	char str_value[NDM_INT_MAX_BUFSIZE(intmax_t)];

	snprintf(str_value, sizeof(str_value), "%ji", value);

	return ndm_xml_node_append_child_str(node, name, str_value);
}

struct ndm_xml_node_t *ndm_xml_node_append_child_uint(
		struct ndm_xml_node_t *node,
		const char *const name,
		const uintmax_t value)
{
	char str_value[NDM_INT_MAX_BUFSIZE(uintmax_t)];

	snprintf(str_value, sizeof(str_value), "%ju", value);

	return ndm_xml_node_append_child_str(node, name, str_value);
}

struct ndm_xml_attr_t *ndm_xml_node_append_attr_str(
		struct ndm_xml_node_t *node,
		const char *const name,
		const char *const value)
{
	struct ndm_xml_document_t *doc = ndm_xml_node_document(node);
	struct ndm_xml_attr_t *new_attr =
		ndm_xml_document_alloc_attr(doc,
			name == NULL ? "" : ndm_xml_document_alloc_str(doc, name),
			value == NULL ? "" : ndm_xml_document_alloc_str(doc, value));

	if (ndm_xml_document_is_valid(doc)) {
		ndm_xml_node_append_attr(node, new_attr);

		return new_attr;
	}

	return NULL;
}

struct ndm_xml_attr_t *ndm_xml_node_append_attr_int(
		struct ndm_xml_node_t *node,
		const char *const name,
		const intmax_t value)
{
	char str_value[NDM_INT_MAX_BUFSIZE(intmax_t)];

	snprintf(str_value, sizeof(str_value), "%ji", value);

	return ndm_xml_node_append_attr_str(node, name, str_value);
}

struct ndm_xml_attr_t *ndm_xml_append_attr_uint(
		struct ndm_xml_node_t *node,
		const char *const name,
		const uintmax_t value)
{
	char str_value[NDM_INT_MAX_BUFSIZE(uintmax_t)];

	snprintf(str_value, sizeof(str_value), "%ju", value);

	return ndm_xml_node_append_attr_str(node, name, str_value);
}

void ndm_xml_node_insert_child(
		struct ndm_xml_node_t *node,
		struct ndm_xml_node_t *where,
		struct ndm_xml_node_t *child)
{
	assert (node->document == child->document);
	assert (where == NULL || where->document == node->document);
	assert (where == NULL || where->parent == node);
	assert (
		child != NULL &&
		child->parent == NULL &&
		child->type != NDM_XML_NODE_TYPE_DOCUMENT);

	if (where == node->first_child) {
		ndm_xml_node_prepend_child(node, child);
	} else
	if (where == NULL) {
		ndm_xml_node_append_child(node, child);
	} else {
		child->prev_sibling = where->prev_sibling;
		child->next_sibling = where;
		where->prev_sibling->next_sibling = child;
		where->prev_sibling = child;
		child->parent = node;
	}
}

void ndm_xml_node_remove_first_child(
		struct ndm_xml_node_t *node)
{
	struct ndm_xml_node_t *child = node->first_child;

	assert (node->first_child != NULL);

	node->first_child = child->next_sibling;

	if (child->next_sibling != NULL) {
		child->next_sibling->prev_sibling = NULL;
	} else {
		node->last_child = NULL;
	}

	child->parent = NULL;
}

void ndm_xml_node_remove_last_child(
		struct ndm_xml_node_t *node)
{
	struct ndm_xml_node_t *child = node->last_child;

	assert (node->first_child != NULL);

	if (child->prev_sibling != NULL) {
		node->last_child = child->prev_sibling;
		child->prev_sibling->next_sibling = NULL;
	} else {
		node->first_child = NULL;
	}

	child->parent = NULL;
}

void ndm_xml_node_remove_child(
		struct ndm_xml_node_t *node,
		struct ndm_xml_node_t *child)
{
	assert (child != NULL && child->parent == node);
	assert (node->first_child != NULL);

	if (child == node->first_child) {
		ndm_xml_node_remove_first_child(node);
	} else
	if (child == node->last_child) {
		ndm_xml_node_remove_last_child(node);
	} else {
		child->prev_sibling->next_sibling = child->next_sibling;
		child->next_sibling->prev_sibling = child->prev_sibling;
		child->parent = NULL;
	}
}

void ndm_xml_node_remove_all_children(
		struct ndm_xml_node_t *node,
		struct ndm_xml_node_t *start_child)
{
	struct ndm_xml_node_t *n =
		(start_child == NULL) ? node->first_child : start_child;

	assert (start_child == NULL || start_child->parent == node);

	while (n != NULL) {
		n->parent = NULL;
		n = n->next_sibling;
	}

	if (start_child == NULL || start_child == node->first_child) {
		node->first_child = NULL;
		node->last_child = NULL;
	} else {
		node->last_child = start_child->prev_sibling;
	}
}

void ndm_xml_node_prepend_attr(
		struct ndm_xml_node_t *node,
		struct ndm_xml_attr_t *attr)
{
	assert (attr != NULL && attr->node == NULL);
	assert (node->document == attr->document);

	if (node->first_attr != NULL) {
		attr->next = node->first_attr;
		node->first_attr->prev = attr;
	} else {
		attr->next = NULL;
		node->last_attr = attr;
	}

	node->first_attr = attr;
	attr->node = node;
	attr->prev = NULL;
}

void ndm_xml_node_append_attr(
		struct ndm_xml_node_t *node,
		struct ndm_xml_attr_t *attr)
{
	assert (attr != NULL && attr->node == NULL);
	assert (node->document == attr->document);

	if (node->first_attr != NULL) {
		attr->prev = node->last_attr;
		node->last_attr->next = attr;
	} else {
		attr->prev = NULL;
		node->first_attr = attr;
	}

	node->last_attr = attr;
	attr->node = node;
	attr->next = NULL;
}

void ndm_xml_node_insert_attr(
		struct ndm_xml_node_t *node,
		struct ndm_xml_attr_t *where,
		struct ndm_xml_attr_t *attr)
{
	assert (where == NULL || where->node == node);
	assert (where == NULL || node->document == where->document);
	assert (attr != NULL && attr->node == NULL);
	assert (node->document == attr->document);

	if (where == node->first_attr) {
		ndm_xml_node_prepend_attr(node, attr);
	} else
	if (where == NULL) {
		ndm_xml_node_append_attr(node, attr);
	} else {
		attr->prev = where->prev;
		attr->next = where;
		where->prev->next = attr;
		where->prev = attr;
		attr->node = node;
	}
}

void ndm_xml_node_remove_first_attr(
		struct ndm_xml_node_t *node)
{
	struct ndm_xml_attr_t *attr = node->first_attr;

	assert (attr != NULL);

	if (attr->next != NULL) {
		attr->next->prev = NULL;
	} else {
		node->last_attr = NULL;
	}

	attr->node = NULL;
	node->first_attr = attr->next;
}

void ndm_xml_node_remove_last_attr(
		struct ndm_xml_node_t *node)
{
	struct ndm_xml_attr_t *attr = node->last_attr;

	assert (node->first_attr != NULL);

	if (attr->prev != NULL) {
		attr->prev->next = NULL;
		node->last_attr = attr->prev;
	} else {
		node->first_attr = NULL;
	}

	attr->node = NULL;
}

void ndm_xml_node_remove_attr(
		struct ndm_xml_node_t *node,
		struct ndm_xml_attr_t *where)
{
	assert (node->first_attr != NULL && where->node == node);

	if (where == node->first_attr) {
		ndm_xml_node_remove_first_attr(node);
	} else
	if (where == node->last_attr) {
		ndm_xml_node_remove_last_attr(node);
	} else {
		where->prev->next = where->next;
		where->next->prev = where->prev;
		where->node = NULL;
	}
}

void ndm_xml_node_remove_all_attr(
		struct ndm_xml_node_t *node)
{
	struct ndm_xml_attr_t *a = node->first_attr;

	while (a != NULL) {
		a->node = NULL;
		a = a->next;
	}

	node->first_attr = node->last_attr = NULL;
}

/**
 * XML attribute functions.
 */

const char *ndm_xml_attr_name(
		const struct ndm_xml_attr_t *attr)
{
	return attr->name;
}

size_t ndm_xml_attr_name_size(
		const struct ndm_xml_attr_t *attr)
{
	return attr->name_size;
}

const char *ndm_xml_attr_value(
		const struct ndm_xml_attr_t *attr)
{
	return attr->value;
}

size_t ndm_xml_attr_value_size(
		const struct ndm_xml_attr_t *attr)
{
	return attr->value_size;
}

void ndm_xml_attr_set_name(
		struct ndm_xml_attr_t *attr,
		const char *const name)
{
	attr->name = (name == NULL) ? "" : name;
	attr->name_size = strlen(attr->name);
}

void ndm_xml_attr_set_value(
		struct ndm_xml_attr_t *attr,
		const char *const value)
{
	attr->value = (value == NULL) ? "" : value;
	attr->value_size = strlen(attr->value);
}

struct ndm_xml_node_t *ndm_xml_attr_node(
		const struct ndm_xml_attr_t *attr)
{
	return attr->node;
}

struct ndm_xml_document_t *ndm_xml_attr_document(
		const struct ndm_xml_attr_t *attr)
{
	return attr->document;
}

struct ndm_xml_attr_t *ndm_xml_attr_next(
		const struct ndm_xml_attr_t *attr,
		const char *const name)
{
	struct ndm_xml_attr_t *a = (attr->node == NULL) ? NULL : attr->next;

	if (!__ndm_xml_name_is_empty(name)) {
		while (a != NULL && strcmp(a->name, name) != 0) {
			a = a->next;
		}
	}

	return a;
}

struct ndm_xml_attr_t *ndm_xml_attr_prev(
		const struct ndm_xml_attr_t *attr,
		const char *const name)
{
	struct ndm_xml_attr_t *a = (attr->node == NULL) ? NULL : attr->prev;

	if (!__ndm_xml_name_is_empty(name)) {
		while (a != NULL && strcmp(a->name, name) != 0) {
			a = a->prev;
		}
	}

	return a;
}

