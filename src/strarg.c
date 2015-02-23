#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ndm/strarg.h>
#include <ndm/string.h>

static void __ndm_strarg_remove_symbol(char *p)
{
	/* move with the null-terminating character */
	memmove(p, p + 1, strlen(p));
}

static int __ndm_strarg_parse_escape_sequence(char **p)
{
	int error = 0;

	if (**p != '\\') {
		/* escape sequence should start with "\" */
		error = EINVAL;
	} else {
		__ndm_strarg_remove_symbol(*p);

		switch (**p) {
			case 'a': {
				**p = '\a';

				break;
			}

			case 'b': {
				**p = '\b';

				break;
			}

			case 'f': {
				**p = '\f';

				break;
			}

			case 'n': {
				**p = '\n';

				break;
			}

			case 'r': {
				**p = '\r';

				break;
			}

			case 't': {
				**p = '\t';

				break;
			}

			case 'v': {
				**p = '\v';

				break;
			}

			case '\\':
			case '\'':
			case '"': {
				/* skip a symbol */

				break;
			}

			default: {
				/*
				 * \xHH (one or two digits),
				 * \nnn (one to three digits),
				 * \e, \E (an escape character),
				 * \cx (control-x sequence)
				 * and invalid sequences are not supported
				 */
				error = EINVAL;
			}
		}

		if (error == 0) {
			(*p)++;
		}
	}

	return error;
}

static int __ndm_strarg_parse_quoted_string(char **p)
{
	char q = **p;
	int error = 0;

	if (q != '\'' && q != '"') {
		/* quoted string should start with " or ' */
		error = EINVAL;
	} else {
		__ndm_strarg_remove_symbol(*p);

		while (**p && **p != q && !error) {
			if (**p == '\\') {
				error = __ndm_strarg_parse_escape_sequence(p);
			} else {
				++(*p);
			}
		}

		if (error == 0) {
			if (**p != q) {
				/* quoted string should end with 'q' */
				error = EINVAL;
			} else {
				__ndm_strarg_remove_symbol(*p);
			}
		}
	}

	return error;
}

bool ndm_strarg_parse(
		const char *const command,
		struct ndm_strvec_t *args)
{
	char **argv = (char **) malloc(sizeof(char *));
	char *s = ndm_string_dup(command);
	int error = errno;

	if (!argv || !s) {
		/* not enough memory */
		error = ENOMEM;
	} else {
		unsigned int argc = 0;
		char *p = s;

		argv[0] = NULL;

		do {
			while (isspace(*p)) {
				++p;
			}

			if (*p && !isspace(*p)) {
				char **arg_set = (char **) realloc(
					argv, (argc + 2)*sizeof(void *));

				if (arg_set == NULL) {
					/* not enough memory */
					error = ENOMEM;
				} else {
					argv = arg_set;
					argv[argc++] = p;
					argv[argc] = NULL;

					while (*p && !isspace(*p) && !error) {
						if (*p == '\'' || *p == '"') {
							error = __ndm_strarg_parse_quoted_string(&p);
						} else
						if (*p == '\\') {
							error = __ndm_strarg_parse_escape_sequence(&p);
						} else {
							++p;
						}
					}
				}
			}

			if (*p && !error) {
				*p = '\0';
				++p;
			}
		} while (*p && error == 0);

		if (error == 0) {
			if (argc == 0) {
				/* empty command string */
				ndm_strvec_clear(args);
			} else
			if (!ndm_strvec_assign_array(args, (const char **) argv)) {
				error = errno;
			}
		}
	}

	free(argv);
	free(s);

	errno = error;

	return (error == 0) ? true : false;
}

