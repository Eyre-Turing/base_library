#include "eyre_turing_lib.h"
#include "eyre_turing_network.h"
#include <iostream>

using namespace std;

#define NOTHING		0
#define TCP_SERVER	1
#define TCP_CLIENT	2
#define UDP			3
#define JSON_TEST	4

#ifndef USE_FOR
#define USE_FOR		NOTHING
#endif

#if (USE_FOR == TCP_SERVER)
#include <vector>
#include <algorithm>

vector<TcpSocket *> clients;
bool running;

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

void onClosed(TcpServer *server)
{
	running = false;
	cout<<"closed."<<endl;
}

int main()
{
	TcpServer tcpServer;
	tcpServer.setNewConnectingCallBack(onNewConnecting);
	tcpServer.setClosedCallBack(onClosed);
	cout<<"input your server port: ";
	unsigned short port;
	cin>>port;
	running = (tcpServer.start(port)==TCP_SERVER_READYTORUN);
	String cmd;
	while(running)
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
			getchar();
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
bool enable;

void onRead(TcpSocket *tcpSocket, ByteArray data)
{
	cout<<"read message:"<<endl<<data.toString(CODEC_UTF8)<<endl;
}

void onDisconnected(TcpSocket *client)
{
	enable = false;
	cout<<"disconnected."<<endl;
}

void onConnected(TcpSocket *client)
{
	cout<<"connected."<<endl;
}

void onConnectError(TcpSocket *client, int errorStatus)
{
	enable = false;
	cout<<"connect error."<<endl;
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
	getchar();
	enable = (tcpSocket.connectToHost(host, port)==TCP_SOCKET_READYTOCONNECT);
	String data;
	while(enable)
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
		getchar();
		getline(cin, data);
		data.replace("\\n", "\n");
		udpSocket.send(ip, port, ByteArray::fromString(data, CODEC_UTF8));
	}
	cout<<"input any thing to quit."<<endl;
	cin>>data;
	return 0;
}
#elif (USE_FOR == JSON_TEST)
int main()
{
	cout << "JsonNone 显示结果为: " << JsonNone << endl;
	cout << "如果对JsonNone 赋值的话: " << endl;
	JsonNone = "hello";
	cout << "JsonNone 显示结果为: " << JsonNone << endl;
	cout << "如果对 JsonNone 添加下标: " << endl;
	JsonNone["test"] = 1.2;
	cout << "JsonNone 显示结果为: " << JsonNone << endl;

	cout << "JsonArrayNone 显示结果为: " << JsonArrayNone << endl;
	Json test = JsonArrayNone;
	test.toArray().append("hello world");
	cout << test << endl;

	Json json;
	cout << "传统赋值方式" << endl;
	json["math"] = JsonNone;
	Json &math = json["math"];
	math["pi"] = 3.14;
	math["e"] = 2.718;
	math["bigshot"] = JsonArrayNone;
	JsonArray bigshot = ((Json &)math["bigshot"]).toArray();
	bigshot.append("牛顿");
	bigshot.append("欧拉");
	bigshot.append("图灵");
	json["web"] = JsonArrayNone;
	JsonArray web = ((Json &)json["web"]).toArray();
	web.append(JsonNone);
	web[0]["url"] = "www.baidu.com";
	web[0]["description"] = "百度一下，你就知道";
	web.append(JsonNone);
	web[1]["url"] = "github.com";
	web[1]["description"] = "一个面向开源及私有软件项目的托管平台";
	cout << json << endl;

	cout << json.toString(true) << endl;
	File file("test.json");
	file.open(FILE_OPEN_MODE_Write);
	file.write(ByteArray::fromString(json.toString(true)));
	file.close();

	cout << "取值" << endl;
	cout << "pi = " << ((Json &)((Json &)json["math"])["pi"]).number() << endl;
	cout << "e = " << ((Json &)json["math"]["e"]).number() << endl;

	cout << "快速赋值方式" << endl;
	json = JsonNone;
	json["author"]["name"]["Chinese"] = "赵振海";
	json["author"]["name"]["English"] = "Eyre Turing";
	json["author"]["age"] = 23.0;
	json["author"]["male"] = true;
	json["author"]["friend"] = JsonArrayNone;
	JsonArray authorFriend = ((Json &)json["author"]["friend"]).toArray();
	authorFriend.append("胡婉茹");
	authorFriend.append("翁朝曦");
	authorFriend.append("龙文汉");
	json["app"]["version"] = "1.0.0.20210921_beta";
	json["app"]["whatsnew"] = "添加了JSON模块";
	cout << json <<endl;

	cout << json.toString() << endl;
	cout << json.toString(true) << endl;

	cout << "取值" << endl;
	cout << "作者中文名: " << ((Json &)json["author"]["name"]["Chinese"]).string() << endl;
	cout << "作者朋友: " << endl;
	for (size_t i = 0; i < ((Json &)json["author"]["friend"]).toArray().size(); ++i)
	{
		cout << "  - " << ((Json &)json["author"]["friend"]).toArray()[i].string() << endl;
	}
	return 0;
}
#else
int main()
{
	return 0;
}
#endif
