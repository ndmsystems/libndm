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

#include "log.h"

#define DOTS						"[...]"
#define ERROR_MSG_MAX_LENGTH		64			/* > sizeof(DOTS)	*/
#define FEEDBACK_MAX_ENV_SIZE	4096
int ndmSocket_wait_for(const int s,const short mode,const unsigned long ms);
unsigned ndmTime_get_monotonic_s();
