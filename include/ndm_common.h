#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/if_ether.h>

#define DOTS						"[...]"
#define ERROR_MSG_MAX_LENGTH		64			/* > sizeof(DOTS)	*/
#define FEEDBACK_MAX_ENV_SIZE	4096

/* sokect */
int ndmSocket_wait_for(const int s,const short mode,const unsigned long ms);

/* feedback */
uint32_t ndmUtil_calculate_checksum(const void *data, const unsigned long octet_count, uint32_t initial);
void ndmFeedback_multienv(const char *const executable, const char *const arg, char *envs, const int env_length);

/* iface */
int ndmIface_get_ipaddr(const char *const iface_name, uint32_t *ipaddr);
int ndmIface_get_mac(const char *const iface_name, uint8_t *hwaddr);
int ndmIface_get_ifindex(const char *const iface_name, int *if_index);

/* utils */
unsigned ndmTime_get_monotonic_s();
char *ndmUtils_strerror(const int error);
void ndm_get_ident(const char *full_path);
uint32_t ndm_get_random();
int ndm_str2long(char *string_value, unsigned long *value);
int ndm_file_exist(const char *const file_name, const int mode);



