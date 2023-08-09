/************************************************************//**
*
*	@file: platform_socket.h
*	@author: Martin Fouilleul
*	@date: 22/03/2019
*	@revision:
*
*****************************************************************/
#ifndef __PLATFORM_SOCKET_H_
#define __PLATFORM_SOCKET_H_

#include<sys/time.h> // timeval
#include"typedefs.h"

#ifdef __cplusplus
extern "C" {
#else
typedef struct timeval timeval;
#endif //__cplusplus


//----------------------------------------------------------------------------------
// Errors
//----------------------------------------------------------------------------------

//TODO(martin): extend these error codes
const int SOCK_ERR_OK	    = 0,
	  SOCK_ERR_UNKNOWN  = -1,
	  SOCK_ERR_ACCESS   = -2,
	  SOCK_ERR_MEM      = -3,
	  SOCK_ERR_INTR	    = -4,
	  SOCK_ERR_USED	    = -5,
	  SOCK_ERR_BADF	    = -6,
	  SOCK_ERR_ABORT    = -7,
	  SOCK_ERR_NBLOCK   = -8;

int SocketGetLastError();
const char* SocketGetLastErrorMessage();

//----------------------------------------------------------------------------------
// Addresses
//----------------------------------------------------------------------------------

//NOTE(martin): net_ip and net_port are stored in network byte order
//              host_ip and host_port are stored in host byte order
typedef uint32 net_ip;
typedef uint16 net_port;
typedef uint32 host_ip;
typedef uint16 host_port;

typedef struct
{
	net_ip ip;
	net_port port;
} socket_address;

//NOTE(martin): these are in host byte order !
const host_ip SOCK_IP_LOOPBACK = 0x7f000001;
const host_ip SOCK_IP_ANY = 0;
const host_port SOCK_PORT_ANY = 0;

net_ip StringToNetIP(const char* addr);
const char* NetIPToString(net_ip ip);

host_ip StringToHostIP(const char* addr);
const char* HostIPToString(host_ip ip);

net_ip HostToNetIP(host_ip ip);
net_port HostToNetPort(host_port port);
host_ip NetToHostIP(net_ip ip);
host_port NetToHostPort(net_port port);


int SocketGetIFAddresses(int* count, net_ip* ips);
net_ip SocketGetDefaultExternalIP();

//----------------------------------------------------------------------------------
// Socket API
//----------------------------------------------------------------------------------

typedef struct platform_socket platform_socket;

typedef enum { SOCK_UDP, SOCK_TCP } socket_transport;

const int SOCK_MSG_OOB		= 0x01,
	  SOCK_MSG_PEEK		= 0x02,
	  SOCK_MSG_DONTROUTE	= 0x04,
	  SOCK_MSG_WAITALL	= 0x40;

platform_socket* SocketOpen(socket_transport transport);
int SocketClose(platform_socket* socket);

int SocketBind(platform_socket* socket, socket_address* addr);
int SocketListen(platform_socket* socket, int backlog);
platform_socket* SocketAccept(platform_socket* socket, socket_address* from);

int SocketConnect(platform_socket* socket, socket_address* addr);

int64 SocketReceive(platform_socket* socket, void* buffer, uint64 size, int flags);
int64 SocketReceiveFrom(platform_socket* socket, void* buffer, uint64 size, int flags, socket_address* from);

int64 SocketSend(platform_socket* socket, void* buffer, uint64 size, int flags);
int64 SocketSendTo(platform_socket* socket, void* buffer, uint64 size, int flags, socket_address* to);

int SocketGetAddress(platform_socket* socket, socket_address* addr);

//----------------------------------------------------------------------------------
// Multiplexing
//----------------------------------------------------------------------------------
const uint8 SOCK_ACTIVITY_IN  = 1<<0,
            SOCK_ACTIVITY_OUT = 1<<2,
	    SOCK_ACTIVITY_ERR = 1<<3;

typedef struct
{
	platform_socket* sock;
	uint8 watch;
	uint8 set;
} socket_activity;

int SocketSelect(uint32 count, socket_activity* set, double timeout);

//----------------------------------------------------------------------------------
// Socket Options
//----------------------------------------------------------------------------------

int SocketSetReceiveTimeout(platform_socket* socket, timeval* tv);
int SocketSetSendTimeout(platform_socket* socket, timeval* tv);
int SocketSetBroadcast(platform_socket* sock, bool enable);
int SocketSetReuseAddress(platform_socket* sock, bool enable);
int SocketSetReusePort(platform_socket* sock, bool enable);
int SocketSetReceiveTimestamping(platform_socket* socket, bool enable);

//----------------------------------------------------------------------------------
// Multicast
//----------------------------------------------------------------------------------

int SocketSetMulticastLoop(platform_socket* sock, bool enable);
int SocketJoinMulticastGroup(platform_socket* socket, host_ip group, host_ip interface);
int SocketLeaveMulticastGroup(platform_socket* socket, host_ip group, host_ip interface);

//----------------------------------------------------------------------------------
//Ancillary data API
//----------------------------------------------------------------------------------
typedef struct
{
	u64 messageBufferSize;
	char* messageBuffer;

	u64 controlBufferSize;
	char* controlBuffer;
} socket_msg;

int SocketReceiveMessage(platform_socket* socket, socket_msg* msg, socket_address* from);

#ifdef __cplusplus
} // extern "C"
#endif


#endif //__PLATFORM_SOCKET_H_
