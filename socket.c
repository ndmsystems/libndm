#include <netinet/if_ether.h>
#include <netpacket/packet.h>
#include <linux/filter.h>
#include "ndm_common.h"
#include "dhcp.h"


/* Both for client and server */
int get_raw_dhcp_socket(int *if_index)
{
	int fd = -1;
	struct sockaddr_ll sock;
	
	
	static const struct sock_filter filter_instr[] = {
        /* check for udp */
        BPF_STMT(BPF_LD|BPF_B|BPF_ABS, 9),
        BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, IPPROTO_UDP, 0, 4),     /* L5, L1, is UDP? */
        /* skip IP header */
        BPF_STMT(BPF_LDX|BPF_B|BPF_MSH, 0),                     /* L5: */
        /* check udp source and destination ports */
        BPF_STMT(BPF_LD|BPF_W|BPF_IND, 0),
        BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, DHCP_CLIENT_SERVER_PORT, 0, 1), /* L3, L4 */
        /* returns */
        BPF_STMT(BPF_RET|BPF_K, 0x0fffffff ),                   /* L3: pass */
        BPF_STMT(BPF_RET|BPF_K, 0),                             /* L4: reject */
    };
    
	static const struct sock_fprog filter_prog = {
        .len = sizeof(filter_instr) / sizeof(filter_instr[0]),
        .filter = (struct sock_filter *) filter_instr,
    };
    
    
    if ((fd = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP))) < 0) {
		ndmLog_error("Unable to open raw socket\n");
	} else {
		if (setsockopt(fd, SOL_SOCKET, SO_ATTACH_FILTER, 
					&filter_prog, sizeof(filter_prog)) < 0)
		{
			ndmLog_warn("Unable to apply filter to raw socket\n");
			/* Ignore error (kernel lack???)*/
		};
		sock.sll_family = AF_PACKET;
		sock.sll_protocol = htons(ETH_P_IP);
		sock.sll_ifindex = *if_index;
		if (bind(fd, (struct sockaddr *) &sock, sizeof(sock)) < 0) {
			ndmLog_error("Unable to bind to raw socket\n");
			close(fd);
			return -1;
		}
		
	}
	
	return fd;
    

}
