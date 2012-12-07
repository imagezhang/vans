#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/net.h>
#include <net/sock.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/in.h>
#include <linux/socket.h>
#include <linux/slab.h>

#include "vans.h"

//============================================================================
struct sockaddr_in vans_out_addr;
struct socket *vans_out_sock;

struct sockaddr_in vans_in_addr;
struct socket *vans_in_sock;

//============================================================================
u32 create_address(u8 *ip);

//============================================================================
int vans_net_init(void)
{
	u8 ip_addr[4] = {224,1,1,1};
	int r;
	
	r = sock_create(PF_INET, SOCK_DGRAM, IPPROTO_UDP, &vans_out_sock);
	DBG_VANS(DBG_VANS_NET, KERN_INFO, "socket created %d", r)

	memset(&vans_out_addr, 0, sizeof(vans_out_addr));
	vans_out_addr.sin_family = AF_INET;
	vans_out_addr.sin_port = htons(20000);
	vans_out_addr.sin_addr.s_addr = htonl(create_address(ip_addr));

	return 0;
}

int vans_net_sendto(void * buff, size_t len)
{
	int r;
	struct kvec kvec;
	struct msghdr msg;

	kvec.iov_base = buff;
	kvec.iov_len = len;

	msg.msg_name = NULL;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_name = &vans_out_addr;
	msg.msg_namelen = sizeof(vans_out_addr);
	msg.msg_flags = MSG_DONTWAIT | MSG_NOSIGNAL;

	r = kernel_sendmsg(vans_out_sock, &msg,
			   &kvec, 1, len);
	DBG_VANS(DBG_VANS_NET, KERN_INFO, "sendmsg %d", r)

	return 0;
}

int vans_net_recv()
{
	return 0;
}

//============================================================================
u32 create_address(u8 *ip)
{
	u32 addr = 0;
	int i;
	for(i = 0;i < 4;++i) {
		addr += ip[i];
		if(i == 3) {
			break;
		}
		addr <<= 8;
	}
	return addr;
}
