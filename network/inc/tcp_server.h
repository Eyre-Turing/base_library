#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#ifdef _WIN32
#include <winsock.h>
#include <queue>
#else
#include <sys/socket.h>
#endif

#include <map>
#include <pthread.h>

#define TCP_SERVER_CLOSED	0
#define TCP_SERVER_RUNNING	1

class TcpSocket;
#include "tcp_socket.h"

class TcpServer
{
public:
	typedef void (*NewConnecting)(TcpServer *server, TcpSocket *client);
	
	TcpServer();
	virtual ~TcpServer();
	
	void start(unsigned short port, int family=AF_INET, unsigned long addr=INADDR_ANY, int backlog=5);
	void abort();
	
	void setNewConnectingCallBack(NewConnecting newConnecting);
	
	class Thread
	{
	public:
		static void *selectThread(void *s);
	};
	
	friend class TcpSocket;
	
private:
#ifdef _WIN32
	WSADATA m_wsadata;
	
	/*
	 * Windows system remove a sockfd need --fd in for loop,
	 * so removeClient() don't remove sockfd, removeClient() append
	 * sockfd which need remove to m_waitForRemoveSockfds, and
	 * selectThread will remove the sockfd.
	 */
	std::queue<int> m_waitForRemoveSockfds;
	pthread_mutex_t m_waitForRemoveSockfdsMutex;
	SOCKET m_sockfd;
#else
	int m_sockfd;
#endif
	int m_runStatus;
	
	fd_set m_readfds;
	pthread_mutex_t m_readfdsMutex;
	
	pthread_t m_listenThread;
	
	NewConnecting m_onNewConnecting;
#ifdef _WIN32
	std::map<SOCKET, TcpSocket *> m_clientMap;
#else
	std::map<int, TcpSocket *> m_clientMap;
#endif
	
	/*
	 * Will create a TcpSocket* which use clientSockfd to send an recv message,
	 * and add pair(clientSockfd, created TcpSocket*) to m_clientMap.
	 * And add clientSockfd to m_readfds.
	 */
#ifdef _WIN32
	TcpSocket *appendClient(SOCKET clientSockfd);
#else
	TcpSocket *appendClient(int clientSockfd);
#endif
	
	/*
	 * Will close clientSockfd, remove clientSockfd from m_readfds
	 * and call back onDisconnected.
	 * Note: this function don't auto delete the TcpSocket*.
	 */
#ifdef _WIN32
	bool removeClient(SOCKET clientSockfd);
#else
	bool removeClient(int clientSockfd);
#endif
};

#endif	//TCP_SERVER_H
