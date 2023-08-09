/************************************************************//**
*
*	@file: posix_socket.c
*	@author: Martin Fouilleul
*	@date: 22/03/2019
*	@revision:
*
*****************************************************************/

#include<sys/socket.h>	// socket()
#include<netinet/ip.h>	// socaddr_in
#include<arpa/inet.h>	// inet_addr()
#include<ifaddrs.h>	// getifaddrs() / freeifaddrs()
#include<unistd.h>	// close()
#include<string.h>	// strerror()
#include<errno.h>	// errno
#include<stdlib.h>	// malloc()/free()

#include"platform_socket.h"
#include"platform_debug.h"

typedef struct in_addr in_addr;
typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;
typedef struct msghdr msghdr;
typedef struct cmsghdr cmsghdr;
typedef struct ip_mreq ip_mreq;
typedef struct iovec iovec;

net_ip StringToNetIP(const char* addr)
{
	return(inet_addr(addr));
}
const char* NetIPToString(net_ip ip)
{
	in_addr in;
	in.s_addr = ip;
	return(inet_ntoa(in));
}

host_ip StringToHostIP(const char* addr)
{
	return(NetToHostIP(StringToNetIP(addr)));
}
const char* HostIPToString(host_ip ip)
{
	return(NetIPToString(HostToNetIP(ip)));
}

net_ip HostToNetIP(uint32 ip)
{
	return(htonl(ip));
}
net_port HostToNetPort(uint16 port)
{
	return(htons(port));
}
uint32 NetToHostIP(net_ip ip)
{
	return(ntohl(ip));
}
uint16 NetToHostPort(net_port port)
{
	return(ntohs(port));
}

static int PlatformToSocketFlags(int flags)
{
	int sflags = 0;
	if(flags & SOCK_MSG_OOB)
	{
		sflags |= MSG_OOB;
	}
	if(flags & SOCK_MSG_PEEK)
	{
		sflags |= MSG_PEEK;
	}
	if(flags & SOCK_MSG_DONTROUTE)
	{
		sflags |= MSG_DONTROUTE;
	}
	if(flags & SOCK_MSG_WAITALL)
	{
		sflags |= MSG_WAITALL;
	}

	return(sflags);
}

static int ErrnoToSocketError(int err)
{
	//TODO(martin): extend these error codes
	switch(err)
	{
		case 0:		return(SOCK_ERR_OK);
		case EACCES:	return(SOCK_ERR_ACCESS);
		case ENOBUFS:
		case ENOMEM:	return(SOCK_ERR_MEM);
		case EINTR:	return(SOCK_ERR_INTR);
		case EADDRINUSE: return(SOCK_ERR_USED);
		case EBADF:	return(SOCK_ERR_BADF);
		case ECONNABORTED: return(SOCK_ERR_ABORT);

		case EWOULDBLOCK:
		#if EAGAIN != EWOULDBLOCK
			case EAGAIN:
		#endif
			return(SOCK_ERR_NBLOCK);

		default:
			return(SOCK_ERR_UNKNOWN);
	}
}

int SocketGetLastError()
{
	return(ErrnoToSocketError(errno));
}

const char* SocketGetLastErrorMessage()
{
	return(strerror(errno));
}

struct platform_socket
{
	int sd;
};

platform_socket* SocketOpen(socket_transport transport)
{
	int sd = 0;
	switch(transport)
	{
		case SOCK_UDP:
			sd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
		break;

		case SOCK_TCP:
			sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
		break;
	}
	if(!sd)
	{
		return(0);
	}

	platform_socket* sock = (platform_socket*)malloc(sizeof(platform_socket));
	if(!sock)
	{
		close(sd);
		return(0);
	}
	sock->sd = sd;
	return(sock);
}

int SocketClose(platform_socket* sock)
{
	if(!sock)
	{
		return(-1);
	}
	int res = close(sock->sd);
	free(sock);
	return(res);
}

int SocketBind(platform_socket* sock, socket_address* addr)
{
	sockaddr_in saddr;
	saddr.sin_addr.s_addr = addr->ip;
	saddr.sin_port = addr->port;
	saddr.sin_family = AF_INET;

	return(bind(sock->sd, (sockaddr*)&saddr, sizeof(saddr)));
}

int SocketListen(platform_socket* sock, int backlog)
{
	return(listen(sock->sd, backlog));
}
platform_socket* SocketAccept(platform_socket* sock, socket_address* from)
{
	sockaddr_in saddr;
	socklen_t saddrSize = sizeof(saddr);
	int sd = accept(sock->sd, (sockaddr*)&saddr, &saddrSize);

	from->ip = saddr.sin_addr.s_addr;
	from->port = saddr.sin_port;

	if(sd <= 0)
	{
		return(0);
	}
	else
	{
		platform_socket* client = (platform_socket*)malloc(sizeof(platform_socket));
		if(!client)
		{
			close(sd);
			return(0);
		}
		client->sd = sd;
		return(client);

	}
}

int SocketConnect(platform_socket* sock, socket_address* addr)
{
	sockaddr_in saddr;
	saddr.sin_addr.s_addr = addr->ip;
	saddr.sin_port = addr->port;
	saddr.sin_family = AF_INET;

	return(connect(sock->sd, (sockaddr*)&saddr, sizeof(saddr)));
}

int64 SocketReceive(platform_socket* sock, void* buffer, uint64 size, int flags)
{
	return(recv(sock->sd, buffer, size, PlatformToSocketFlags(flags)));
}

int64 SocketReceiveFrom(platform_socket* sock, void* buffer, uint64 size, int flags, socket_address* from)
{
	sockaddr_in saddr;
	socklen_t saddrSize = sizeof(saddr);

	int res = recvfrom(sock->sd, buffer, size, PlatformToSocketFlags(flags), (sockaddr*)&saddr, &saddrSize);

	from->ip = saddr.sin_addr.s_addr;
	from->port = saddr.sin_port;
	return(res);
}

int64 SocketSend(platform_socket* sock, void* buffer, uint64 size, int flags)
{
	return(send(sock->sd, buffer, size, PlatformToSocketFlags(flags)));
}
int64 SocketSendTo(platform_socket* sock, void* buffer, uint64 size, int flags, socket_address* to)
{
	sockaddr_in saddr;
	saddr.sin_addr.s_addr = to->ip;
	saddr.sin_port = to->port;
	saddr.sin_family = AF_INET;

	return(sendto(sock->sd, buffer, size, PlatformToSocketFlags(flags), (sockaddr*)&saddr, sizeof(saddr)));
}


int SocketSetReceiveTimeout(platform_socket* sock, timeval* tv)
{
	DEBUG_ASSERT(sock);
	DEBUG_ASSERT(sock->sd);
	return(setsockopt(sock->sd, SOL_SOCKET, SO_RCVTIMEO, tv, sizeof(timeval)));
}

int SocketSetSendTimeout(platform_socket* sock, timeval* tv)
{
	DEBUG_ASSERT(sock);
	DEBUG_ASSERT(sock->sd);
	return(setsockopt(sock->sd, SOL_SOCKET, SO_SNDTIMEO, tv, sizeof(timeval)));
}

int SocketSetReceiveTimestamping(platform_socket* socket, bool enable)
{
	int opt = enable ? 1 : 0;
	socklen_t len = sizeof(int);
	return(setsockopt(socket->sd, SOL_SOCKET, SO_TIMESTAMP, &enable, len));
}

int SocketSetBroadcast(platform_socket* sock, bool enable)
{
	DEBUG_ASSERT(sock);
	DEBUG_ASSERT(sock->sd);

	int opt = enable ? 1 : 0;
	return(setsockopt(sock->sd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(int)));
}


int SocketSelect(uint32 count, socket_activity* set, double timeout)
{
	fd_set fdInSet;
	fd_set fdOutSet;
	fd_set fdErrSet;
	FD_ZERO(&fdInSet);
	FD_ZERO(&fdOutSet);
	FD_ZERO(&fdErrSet);

	int maxSd = -1;
	for(int i=0; i<count; i++)
	{
		socket_activity* item = &(set[i]);
		if(item->sock)
		{
			item->set = 0;
			if(item->watch & SOCK_ACTIVITY_IN)
			{
				FD_SET(item->sock->sd, &fdInSet);
			}
			if(item->watch & SOCK_ACTIVITY_OUT)
			{
				FD_SET(item->sock->sd, &fdOutSet);
			}
			if(item->watch & SOCK_ACTIVITY_ERR)
			{
				FD_SET(item->sock->sd, &fdErrSet);
			}

			if(item->watch && (item->sock->sd > maxSd))
			{
				maxSd = item->sock->sd;
			}
		}
	}

	if(maxSd <= 0)
	{
		return(0);
	}
	timeval tv;
	tv.tv_sec = (time_t)timeout;
	tv.tv_usec = (suseconds_t)((timeout - tv.tv_sec)*1000000);

	timeval* ptv = timeout >= 0 ? &tv : 0;

	int activity = select(maxSd+1, &fdInSet, &fdOutSet, &fdErrSet, ptv);
	if(activity < 0)
	{
		return(-1);
	}

	int processed = 0;
	for(int i=0; i<count; i++)
	{
		if(processed >= activity)
		{
			break;
		}
		socket_activity* item = &(set[i]);
		if(item->sock)
		{
			if(FD_ISSET(item->sock->sd, &fdInSet))
			{
				item->set |= SOCK_ACTIVITY_IN;
			}
			if(FD_ISSET(item->sock->sd, &fdOutSet))
			{
				item->set |= SOCK_ACTIVITY_OUT;
			}
			if(FD_ISSET(item->sock->sd, &fdErrSet))
			{
				item->set |= SOCK_ACTIVITY_ERR;
			}
			if(item->set)
			{
				processed++;
			}
		}
	}
	return(activity);
}


int SocketGetAddress(platform_socket* sock, socket_address* addr)
{
	sockaddr_in saddr;
	socklen_t sockLen = sizeof(saddr);
	if(getsockname(sock->sd, (sockaddr*)&saddr, &sockLen))
	{
		return(-1);
	}
	addr->ip = saddr.sin_addr.s_addr;
	addr->port = saddr.sin_port;

	return(0);
}


int SocketGetIFAddresses(int* count, net_ip* ips)
{
	struct ifaddrs* ifaList = 0;
	if(getifaddrs(&ifaList))
	{
		return(-1);
	}

	int maxCount = *count;
	int i = 0;

	for(struct ifaddrs* ifa = ifaList; ifa != 0 ; ifa = ifa->ifa_next)
	{
		if(i >= maxCount)
		{
			freeifaddrs(ifaList);
			*count = i;
			return(-1);
		}
		if(ifa->ifa_addr->sa_family == AF_INET)
		{
			struct in_addr in = ((sockaddr_in*)ifa->ifa_addr)->sin_addr;
			ips[i] = in.s_addr;
			i++;
		}
	}
	freeifaddrs(ifaList);
	*count = i;
	return(0);
}


int SocketSetReuseAddress(platform_socket* sock, bool enable)
{
	int reuse = enable ? 1 : 0;
	return(setsockopt(sock->sd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)));
}

int SocketSetReusePort(platform_socket* sock, bool enable)
{
	int reuse = enable ? 1 : 0;
	return(setsockopt(sock->sd, SOL_SOCKET, SO_REUSEPORT, (char*)&reuse, sizeof(reuse)));
}

int SocketSetMulticastLoop(platform_socket* sock, bool enable)
{
	int on = enable ? 1 : 0;
	return(setsockopt(sock->sd, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&on, sizeof(on)));
}

int SocketJoinMulticastGroup(platform_socket* sock, host_ip group, host_ip interface)
{
	ip_mreq mreq;
	mreq.imr_multiaddr.s_addr = HostToNetIP(group);
	mreq.imr_interface.s_addr = HostToNetIP(interface);
	return(setsockopt(sock->sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)));
}

int SocketLeaveMulticastGroup(platform_socket* sock, host_ip group, host_ip interface)
{
	ip_mreq mreq;
	mreq.imr_multiaddr.s_addr = HostToNetIP(group);
	mreq.imr_interface.s_addr = HostToNetIP(interface);
	return(setsockopt(sock->sd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&mreq, sizeof(mreq)));
}


net_ip SocketGetDefaultExternalIP()
{
	//NOTE(martin): get the default local ip. This is a dumb way to do this
	//              'cause I can't be bothered to think of a better way right now :(

	socket_address addr = {.ip = StringToNetIP("8.8.8.8"),
			       .port = HostToNetPort(5001)};

	platform_socket* sock = SocketOpen(SOCK_UDP);
	if(!sock)
	{
		log_error("can't create socket");
		return(0);
	}

	if(SocketConnect(sock, &addr) != 0)
	{
		log_error("can't connect socket: %s\n", SocketGetLastErrorMessage());
		log_warning("try loopback interface\n");

		addr.ip = HostToNetIP(SOCK_IP_LOOPBACK);
		if(SocketConnect(sock, &addr) != 0)
		{
			log_error("can't connect socket: %s\n", SocketGetLastErrorMessage());
			SocketClose(sock);
			return(0);
		}
	}
	SocketGetAddress(sock, &addr);
	SocketClose(sock);
	return(addr.ip);
}

int SocketReceiveMessage(platform_socket* socket, socket_msg* msg, socket_address* from)
{
	sockaddr_in saddr;

	iovec iov;
	iov.iov_base = msg->messageBuffer;
	iov.iov_len = msg->messageBufferSize;

	msghdr hdr;
	hdr.msg_name = &saddr;
	hdr.msg_namelen = sizeof(saddr);
	hdr.msg_iov = &iov;
	hdr.msg_iovlen = 1;
	hdr.msg_control = msg->controlBuffer;
	hdr.msg_controllen = msg->controlBufferSize;

	int size = recvmsg(socket->sd, &hdr, 0);
	if(size <= 0)
	{
		return(size);
	}

	from->ip = saddr.sin_addr.s_addr;
	from->port = saddr.sin_port;

	msg->controlBufferSize = hdr.msg_controllen;
	msg->messageBufferSize = iov.iov_len;

	return(size);
}
