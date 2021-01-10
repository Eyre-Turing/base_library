#include "eyre_turing_lib.h"
#include "eyre_turing_network.h"
#include <iostream>

using namespace std;

#define NOTHING		0
#define TCP_SERVER	1
#define TCP_CLIENT	2
#define UDP			3

#ifndef USE_FOR
#define USE_FOR		NOTHING
#endif

#if (USE_FOR == TCP_SERVER)
#include <vector>
#include <algorithm>

vector<TcpSocket *> clients;

void onRead(TcpSocket *client, ByteArray data)
{
	vector<TcpSocket *>::iterator it = find(clients.begin(), clients.end(), client);
	if(it == clients.end())
	{
		cout<<"message from client: "<<client<<", id: no find:"<<endl<<data.toString(CODEC_UTF8)<<endl;
	}
	cout<<"message from client: "<<client<<", id: "<<it-clients.begin()<<":"<<endl<<data.toString(CODEC_UTF8)<<endl;
}

void onDisconnected(TcpSocket *client)
{
	vector<TcpSocket *>::iterator it = find(clients.begin(), clients.end(), client);
	if(it == clients.end())
	{
		cout<<"client disconnected: "<<client<<", id: no find."<<endl;
	}
	cout<<"client disconnected: "<<client<<", id: "<<it-clients.begin()<<"."<<endl;
	clients.erase(it);
}

void onNewConnecting(TcpServer *server, TcpSocket *client)
{
	client->setReadCallBack(onRead);
	client->setDisconnectedCallBack(onDisconnected);
	clients.push_back(client);
	cout<<"new client connected: "<<client<<", id: "<<clients.size()-1<<", ip: "<<client->getPeerIp()<<", port: "<<client->getPeerPort()<<endl;
}

int main()
{
	TcpServer tcpServer;
	tcpServer.setNewConnectingCallBack(onNewConnecting);
	cout<<"input your server port: ";
	unsigned short port;
	cin>>port;
	tcpServer.start(port);
	String cmd;
	while(tcpServer.runStatus() == TCP_SERVER_RUNNING)
	{
		cin>>cmd;
		if(cmd == "help")
		{
			cout<<"you can input: stop, send or ls.\n"
					"stop for stop server\n"
					"send for send message to client\n"
					"  send [client id] [data]\n"
					"ls for show clients list"<<endl;
		}
		else if(cmd == "stop")
		{
			tcpServer.abort();
			break;
		}
		else if(cmd == "send")
		{
			unsigned int id;
			cin>>id;
			String data;
			getline(cin, data);
			data.replace("\\n", "\n");
			if(id < clients.size())
			{
				clients[id]->write(ByteArray::fromString(data, CODEC_UTF8));
			}
		}
		else if(cmd == "ls")
		{
			int size = clients.size();
			for(int i=0; i<size; ++i)
			{
				cout<<"id: "<<i<<", client: "<<clients[i]<<endl;
			}
		}
	}
	cout<<"input any thing to quit."<<endl;
	cin>>cmd;
	return 0;
}
#elif (USE_FOR == TCP_CLIENT)
void onRead(TcpSocket *tcpSocket, ByteArray data)
{
	cout<<"read message:"<<endl<<data.toString(CODEC_UTF8)<<endl;
}

void onDisconnected(TcpSocket *client)
{
	cout<<"disconnected."<<endl;
}

void onConnected(TcpSocket *client)
{
	cout<<"connected."<<endl;
}

int main()
{
	TcpSocket tcpSocket;
	tcpSocket.setReadCallBack(onRead);
	tcpSocket.setDisconnectedCallBack(onDisconnected);
	tcpSocket.setConnectedCallBack(onConnected);
	cout<<"input server ip: ";
	String host;
	cin>>host;
	cout<<"input server port: ";
	unsigned short port;
	cin>>port;
	tcpSocket.connectToHost(host, port);
	String data;
	while(tcpSocket.connectStatus() == TCP_SOCKET_CONNECTED)
	{
		getline(cin, data);
		data.replace("\\n", "\n");
		if(data == "disconnect")
		{
			tcpSocket.abort();
			break;
		}
		tcpSocket.write(ByteArray::fromString(data, CODEC_UTF8));
	}
	cout<<"input any thing to quit."<<endl;
	cin>>data;
	return 0;
}
#elif (USE_FOR == UDP)
void onRead(UdpSocket *udpSocket, String ip, unsigned short port, ByteArray data)
{
	cout<<"recive from ip: "<<ip<<", port: "<<port<<":"<<endl<<data.toString(CODEC_UTF8)<<endl;
}

int main()
{
	UdpSocket udpSocket;
	udpSocket.setReadCallBack(onRead);
	cout<<"input your port: ";
	unsigned short port;
	cin>>port;
	udpSocket.start(port);
	String ip, data;
	while(udpSocket.isBound())
	{
		cin>>ip;
		if(ip == "quit")
		{
			udpSocket.unbind();
			break;
		}
		cin>>port;
		getline(cin, data);
		data.replace("\\n", "\n");
		udpSocket.send(ip, port, ByteArray::fromString(data, CODEC_UTF8));
	}
	cout<<"input any thing to quit."<<endl;
	cin>>data;
	return 0;
}
#else
int main()
{
	return 0;
}
#endif
