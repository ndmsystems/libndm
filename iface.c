#include "ndm_common.h"


/* Caller must memseting it before call */
static int __ifr_ioctl(int request,struct ifreq *ifr)
{
	int s = -1;
	int ret = -1;
	
	if ((s = socket(PF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
			ndmLog_error("Failed to create a raw socket: %s.\n",
				ndm_strerror(errno));
	} else {
		ret = ioctl(s,request,ifr);
		close(s);
	}
	
	return ret;
}

int ndmIface_get_ifindex(const char *const iface_name, int *if_index)
{
	int s = -1;
	struct ifreq ifr;
	int valid = 0;
	
	if ((iface_name != NULL) && (strlen(iface_name) < IFNAMSIZ)) {
		memset(&ifr, 0, sizeof(ifr));
		snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", iface_name);
 
		if (__ifr_ioctl(SIOCGIFINDEX, &ifr) != 0) {
			ndmLog_error(
				"Failed to get \"%s\" interface index: %s.\n",
				iface_name, ndm_strerror(errno));
		}
		else {
			valid = 1;	
			*if_index = ifr.ifr_ifindex;
		}
	}
	return valid;
}

int ndmIface_get_mac(const char *const iface_name, uint8_t *hwaddr)
{
	int s = -1;
	struct ifreq ifr;
	int valid = 0;
	
	if ((iface_name != NULL) && (strlen(iface_name) < IFNAMSIZ) &&
		hwaddr) 
	{
		memset(&ifr, 0, sizeof(ifr));
		snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", iface_name);

		if (__ifr_ioctl(SIOCGIFHWADDR, &ifr) != 0) {
			ndmLog_error(
				"Failed to get \"%s\" interface index: %s.\n",
						iface_name, ndm_strerror(errno));
		}
		else {
			memcpy(hwaddr, ifr.ifr_hwaddr.sa_data, 6);
			valid = 1;
		}		
	}
	return valid;	
}
