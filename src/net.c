#include <ctype.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <ndm/net.h>

#define NDM_NET_DOMAIN_MIN_LEN_			1
#define NDM_NET_DOMAIN_MAX_LEN_			253

#define NDM_NET_SUBDOMAIN_MAX_LEN_		63

bool ndm_net_is_domain_name(const char *const name)
{
	// ^([a-zA-Z0-9]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?\.) ->
	// -> ([a-zA-Z0-9]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9]))$

	size_t name_size = strlen(name);
	bool valid = false;

	if (name_size >= NDM_NET_DOMAIN_MIN_LEN_ &&
		name_size <= NDM_NET_DOMAIN_MAX_LEN_)
	{
		size_t i = (size_t) -1;

		valid = true;

		do {
			/* A subdomain name should start with
			 * an alphanumeric character. */

			++i;

			if (!isalnum(name[i++])) {
				valid = false;
			} else {
				/* the subdomain name should contain
				 * only alphanumeric characters and '-' symbols.
				 * It should end with an alphanumeric character
				 * and to be shorter than @c SUBDOMAIN_MAX_LEN_. */

				const size_t s = i - 1;

				while (
					(isalnum(name[i]) || name[i] == '-') &&
					i < name_size)
				{
					++i;
				}

				valid =
					isalnum(name[i - 1]) &&
					(name[i] == '.' || i == name_size) &&
					i - s <= NDM_NET_SUBDOMAIN_MAX_LEN_;
			}
		} while (valid && i < name_size);
	}

	return valid;
}

