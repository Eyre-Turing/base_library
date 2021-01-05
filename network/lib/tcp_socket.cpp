/*
 * Class TcpSocket can connect to server and send, recive data.
 * The call back function `Read` will exec in a subthread.
 *
 * Author: Eyre Turing.
 * Last edit: 2021-01-03 19:50.
 */

#include "tcp_socket.h"

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
		perror("connectThread error");
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
		else
		{
			tcpSocket->m_connectStatus = TCP_SOCKET_DISCONNECTED;
			if(tcpSocket->m_onDisconnected)
			{
				tcpSocket->m_onDisconnected(tcpSocket);
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
	
	m_connectStatus = TCP_SOCKET_DISCONNECTED;
	
#ifdef _WIN32
	if(WSAStartup(MAKEWORD(1, 1), &m_wsadata) == SOCKET_ERROR)
	{
		fprintf(stderr, "TcpSocket(%p) WSAStartup() fail!\n", this);
	}
	recvBuffer = NULL;
#endif
}

TcpSocket::TcpSocket(TcpServer *server, int sockfd) : m_server(server), m_sockfd(sockfd)
{
	m_res = NULL;
	
	m_onDisconnected = NULL;
	m_onConnected = NULL;
	m_onRead = NULL;
	
	m_connectStatus = TCP_SOCKET_CONNECTED;
	
#ifdef _WIN32
	int optLen = sizeof(recvBufferSize);
	getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char *) &recvBufferSize, &optLen);
	recvBuffer = (char *) malloc(recvBufferSize+1);
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
}

void TcpSocket::connectToHost(const char *addr, unsigned short port, int family)
{
	if(m_server)
	{
		return ;
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
	}
	m_sockfd = socket(m_res->ai_family, m_res->ai_socktype, m_res->ai_protocol);
	if(m_sockfd < 0)
	{
		fprintf(stderr, "TcpSocket(%p)::connectToHost socket() error: %s\n",
				this, strerror(errno));
	}
	
	pthread_create(&m_connectThread, NULL, TcpSocket::Thread::connectThread, this);
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
