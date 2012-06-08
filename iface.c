#include "ndm_common.h"

int ndm_get_ifindex(const char *const iface_name, int *if_index)
{
	int s = -1;
	struct ifreq ifr;
	int valid = 0;
	
	if ((iface_name != NULL) && 
		(s = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
		fprintf(stderr, "Failed to create a raw socket: %s.\n",
			ndm_strerror(errno));
	} else {
		memset(&ifr, 0, sizeof(ifr));
		snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", iface_name);

		if (strlen(iface_name) != strlen(ifr.ifr_name)) {
			fprintf(stderr, "Interface name \"%s\"" \
				"is too long.\n", iface_name);
			valid = 0;
		} else 
		if (ioctl(s, SIOCGIFINDEX, &ifr) != 0) {
				fprintf(stderr,
					"Failed to get \"%s\" interface index: %s.\n",
					iface_name, ndm_strerror(errno));
				valid = 0;
		}
		else valid = 1;
			
	}
	
	if (valid)
		*if_index = ifr.ifr_ifindex;
	
	return valid;
}
