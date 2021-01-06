/*
 * Class TcpSocket can connect to server and send, recive data.
 * The call back function `Read` will exec in a subthread.
 *
 * Author: Eyre Turing.
 * Last edit: 2021-01-06 14:12.
 */

#include "tcp_socket.h"
#include "debug_settings.h"

#ifdef _WIN32
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <netdb.h>
#endif	//_WIN32

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "eyre_string.h"

#define ONCE_READ	1024

void *TcpSocket::Thread::connectThread(void *s)
{
	TcpSocket *tcpSocket = (TcpSocket *) s;
	
	if(connect(tcpSocket->m_sockfd, tcpSocket->m_res->ai_addr, tcpSocket->m_res->ai_addrlen) < 0)
	{
		int errorStatus = errno;
		fprintf(stderr, "connectThread error: %s\n", strerror(errorStatus));
		if(tcpSocket->m_onConnectError)
		{
			tcpSocket->m_onConnectError(tcpSocket, errorStatus);
		}
	}
	else
	{
		tcpSocket->m_connectStatus = TCP_SOCKET_CONNECTED;
		
		pthread_create(&(tcpSocket->m_readThread), NULL,
						TcpSocket::Thread::readThread, s);
		
		if(tcpSocket->m_onConnected)
		{
			tcpSocket->m_onConnected(tcpSocket);
		}
	}
	
	return NULL;
}

void *TcpSocket::Thread::readThread(void *s)
{
	TcpSocket *tcpSocket = (TcpSocket *) s;
	char buffer[ONCE_READ];
	int size;
	while(tcpSocket->m_connectStatus == TCP_SOCKET_CONNECTED)
	{
		size = recv(tcpSocket->m_sockfd, buffer, ONCE_READ, 0);
		if(size > 0)
		{
			if(tcpSocket->m_onRead)
			{
				tcpSocket->m_onRead(tcpSocket, ByteArray(buffer, size));
			}
		}
		else if(size == 0)
		{
			tcpSocket->m_connectStatus = TCP_SOCKET_DISCONNECTED;
			if(tcpSocket->m_onDisconnected)
			{
				tcpSocket->m_onDisconnected(tcpSocket);
			}
		}
		else
		{
			int recvErrno = errno;
#if NETWORK_DETAIL
			fprintf(stdout, "tcpSocket(%p) recv size: %d, errno: %d.\n", tcpSocket, size, recvErrno);
#endif 
			if(recvErrno == 0 || recvErrno == EINTR || recvErrno == EWOULDBLOCK || recvErrno == EAGAIN)
			{
#if NETWORK_DETAIL
				fprintf(stdout, "tcpSocket(%p) recv timeout.\n", tcpSocket);
#endif
			}
			else
			{
				tcpSocket->m_connectStatus = TCP_SOCKET_DISCONNECTED;
				if(tcpSocket->m_onDisconnected)
				{
					tcpSocket->m_onDisconnected(tcpSocket);
				}
			}
		}
	}
	return NULL;
}

TcpSocket::TcpSocket()
{
	m_res = NULL;
	m_server = NULL;
	
	m_onDisconnected = NULL;
	m_onConnected = NULL;
	m_onRead = NULL;
	m_onConnectError = NULL;
	
	m_connectStatus = TCP_SOCKET_DISCONNECTED;
	
#ifdef _WIN32
	if(WSAStartup(MAKEWORD(1, 1), &m_wsadata) == SOCKET_ERROR)
	{
		fprintf(stderr, "TcpSocket(%p) WSAStartup() fail!\n", this);
	}
	recvBuffer = NULL;
#endif

#if NETWORK_DETAIL
	fprintf(stdout, "TcpSocket(%p) created as a client.\n", this);
#endif
}

TcpSocket::TcpSocket(TcpServer *server, int sockfd) : m_server(server), m_sockfd(sockfd)
{
	m_res = NULL;
	
	m_onDisconnected = NULL;
	m_onConnected = NULL;
	m_onRead = NULL;
	m_onConnectError = NULL;
	
	m_connectStatus = TCP_SOCKET_CONNECTED;
	
#ifdef _WIN32
	int optLen = sizeof(recvBufferSize);
	getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char *) &recvBufferSize, &optLen);
	recvBuffer = (char *) malloc(recvBufferSize+1);
#endif

#if NETWORK_DETAIL
	fprintf(stdout, "TcpSocket(%p) created as a server(%p)\'s socket.\n", this, server);
#endif
}

TcpSocket::~TcpSocket()
{
	abort();
#ifdef _WIN32
	if(m_server)
	{
		free(recvBuffer);
	}
	else
	{
		WSACleanup();
	}
#endif

#if NETWORK_DETAIL
	fprintf(stdout, "TcpSocket(%p) destroyed.\n", this);
#endif
}

int TcpSocket::connectToHost(const char *addr, unsigned short port, int family)
{
	if(m_server)
	{
		fprintf(stderr, "warning: this(%p) is a server socket, can not connect to other server!\n", this);
		return TCP_SOCKET_ISSERVER_ERROR;
	}
	abort();
	struct addrinfo hints = {0};
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = family;
	int getErr = getaddrinfo(addr, String::fromNumber(port), &hints, &m_res);
	if(getErr)
	{
		fprintf(stderr, "TcpSocket(%p)::connectToHost getaddrinfo() error: %s\n",
				this, gai_strerror(getErr));
		return TCP_SOCKET_GETADDRINFO_ERROR;
	}
	m_sockfd = socket(m_res->ai_family, m_res->ai_socktype, m_res->ai_protocol);
	if(m_sockfd < 0)
	{
		fprintf(stderr, "TcpSocket(%p)::connectToHost socket() error: %s\n",
				this, strerror(errno));
		return TCP_SOCKET_SOCKETFD_ERROR;
	}
	
#ifdef _WIN32
	int timeout = NETWORK_TIMEOUT*1000;
#else
	struct timeval timeout;
	timeout.tv_sec = NETWORK_TIMEOUT;
	timeout.tv_usec = 0;
#endif
	
	if(setsockopt(m_sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout)) < 0)
	{
		fprintf(stderr, "TcpSocket(%p) set no block error!\n", this);
		return TCP_SOCKET_SETSOCKOPT_ERROR;
	}
	
	pthread_create(&m_connectThread, NULL, TcpSocket::Thread::connectThread, this);
	
	return TCP_SOCKET_READYTOCONNECT;
}

void TcpSocket::abort()
{
	if(m_connectStatus == TCP_SOCKET_DISCONNECTED)
	{
		return ;
	}
	
	if(m_res)
	{
		freeaddrinfo(m_res);
	}
	
	if(m_server)
	{
		m_server->removeClient(m_sockfd);
	}
	else
	{
#ifdef _WIN32
		closesocket(m_sockfd);
#else
		close(m_sockfd);
#endif	//_WIN32

		m_connectStatus = TCP_SOCKET_DISCONNECTED;
		if(m_onDisconnected)
		{
			m_onDisconnected(this);
		}
	}
#if NETWORK_DETAIL
	fprintf(stdout, "TcpSocket(%p) abort.\n", this);
#endif
}

void TcpSocket::setDisconnectedCallBack(Disconnected disconnected)
{
	m_onDisconnected = disconnected;
}

void TcpSocket::setConnectedCallBack(Connected connected)
{
	m_onConnected = connected;
}

void TcpSocket::setReadCallBack(Read read)
{
	m_onRead = read;
}

void TcpSocket::setConnectErrorCallBack(ConnectError connectError)
{
	m_onConnectError = connectError;
}

void TcpSocket::write(const ByteArray &data) const
{
	write(data, data.size());
}

void TcpSocket::write(const char *data, unsigned int size) const
{
	if(size == 0xffffffff)
	{
		size = strlen(data);
	}
	send(m_sockfd, data, size, 0);
}

int TcpSocket::connectStatus() const
{
	return m_connectStatus;
}

String TcpSocket::getPeerIp() const
{
	struct sockaddr_in addr;
#ifdef _WIN32
	int len = sizeof(addr);
#else
	unsigned int len = sizeof(addr);
#endif
	if(getpeername(m_sockfd, (struct sockaddr *) &addr, &len) == 0)
	{
		return String(inet_ntoa(addr.sin_addr), CODEC_UTF8);
	}
	else
	{
		return "";
	}
}

unsigned short TcpSocket::getPeerPort() const
{
	struct sockaddr_in addr;
#ifdef _WIN32
	int len = sizeof(addr);
#else
	unsigned int len = sizeof(addr);
#endif
	if(getpeername(m_sockfd, (struct sockaddr *) &addr, &len) == 0)
	{
		return ntohs(addr.sin_port);
	}
	else
	{
		return 0;
	}
}
