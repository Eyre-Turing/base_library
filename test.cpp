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
#define USE_FOR		JSON_TEST
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
	String::setAutoCodec(CODEC_UTF8);			// 代码保存编码为utf-8，Linux默认就是utf-8，为了解决Windows中文乱码，设置默认String输入编码为utf-8
	cout << String("基本常量输出:") << endl;	// 因为本代码文件保存的编码是utf-8，用String包裹中文字符串可以解决Windows下中文乱码问题
	cout << "JsonNone: " << JsonNone << endl;
	cout << "JsonArrayNone: " << JsonArrayNone << endl;
	cout << "JsonNone.isNull(): " << JsonNone.isNull() << endl;
	cout << String("对常量进行赋值，检查对常量赋值的无效性:") << endl;
	JsonNone = 1;
	cout << "执行 JsonNone = 1 后 JsonNone: " << JsonNone << endl;
	JsonNone["a"] = 1;
	cout << "执行 JsonNone[\"a\"] = 1 后 JsonNone: " << JsonNone << endl;
	JsonNone["a"]["b"] = 1;
	cout << "执行 JsonNone[\"a\"][\"b\"] = 1 后 JsonNone: " << JsonNone << endl;

	cout << String("JSON赋值测试") << endl;
	Json json = JsonNone;
	cout << json << endl;
	cout << json.isNull() << endl;
	json = 1;
	cout << json << endl;
	json["pi"] = 3.14;	// 因为之前设置json = 1时，会把json的类型设置为JSON_NUMBER，所以这步操作必定是失败的
	cout << json << endl;
	json.asObject();
	json["math"] = "数学";	// 因为之前设置json = 1时，会把json的类型设置为JSON_NUMBER，所以这步操作必定是失败的
	cout << json << endl;
	json["math"]["pi"] = 3.14;	// 强行赋值（方括号超过一个就会使用强行赋值）,如果发生冲突（比如这里math字段是字符串类型，但是这个操作会强行把math字段转成JSON_NONE类型）
	cout << json << endl;

	cout << String("复杂JSON赋值") << endl;
	json = JsonNone;
	json["author"]["github-name"] = "Eyre Turing";
	json["author"]["birthday"] = "不告诉你";
	json["author"]["age"] = "23";
	json["author"]["be-good-at"] = JsonArrayNone;
	JsonArray be_good_at = json["author"]["be-good-at"].toArray();
	be_good_at.append("C");
	be_good_at.append("C++");
	be_good_at.append("Go");
	be_good_at.append("Python");
	be_good_at.append("Java");
	json["com"]["github"]["www"]["description"] = "github";
	json["com"]["github"]["www"]["code-url"] = "https://github.com/Eyre-Turing/base_library";
	cout << json << endl;

	cout << String("不美化") << endl;
	cout << json.toString() << endl;
	cout << String("美化") << endl;
	cout << json.toString(true) << endl;
	return 0;
}
#else
int main()
{
	return 0;
}
#endif
