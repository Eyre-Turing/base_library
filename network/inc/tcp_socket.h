#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

/*
 * For send and recive serve's message easily.
 * All message is ByteArray, so need Eyre Turing lib framework.
 * Compile need -lpthread.
 * MinGW compile need -lws2_32.
 *
 * Author: Eyre Turing.
 * Last edit: 2021-01-02 17:10.
 */

#ifdef _WIN32
#include <winsock.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#endif	//_WIN32

#include <pthread.h>
#include "byte_array.h"

#define TCP_SOCKET_DISCONNECTED	0
#define TCP_SOCKET_CONNECTED	1

class TcpSocket
{
public:
	typedef void (*Disconnected)(TcpSocket *s);
	typedef void (*Connected)(TcpSocket *s);
	typedef void (*Read)(TcpSocket *s, ByteArray data);
	
	TcpSocket();
	virtual ~TcpSocket();
	
	void connectToHost(const char *addr, unsigned int port, int family=AF_INET);
	void abort();
	
	void write(const ByteArray &data) const;
	void write(const char *data, unsigned int size=0xffffffff) const;
	
	void setDisconnectedCallBack(Disconnected disconnected);
	void setConnectedCallBack(Connected connected);
	void setReadCallBack(Read read);
	
	int connectStatus() const;
	
	class Thread
	{
	public:
		static void *connectThread(void *s);
		static void *readThread(void *s);
	};
	
private:
#ifdef _WIN32
	WSADATA m_wsadata;
#endif
	int m_sockfd;
	struct addrinfo *m_res;
	int m_connectStatus;
	
	pthread_t m_connectThread;
	pthread_t m_readThread;
	pthread_t m_writeThread;
	
	Disconnected m_onDisconnected;
	Connected m_onConnected;
	Read m_onRead;
};

#endif	//TCP_SOCKET_H 