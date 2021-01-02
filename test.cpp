#include "eyre_turing_lib.h"
#include "tcp_socket.h"
#include <iostream>

using namespace std;

void onRead(TcpSocket *s, ByteArray data)
{
	cout<<"message: "<<data.toString(CODEC_UTF8)<<endl;
}

void onDisconnected(TcpSocket *s)
{
	cout<<"disconnected"<<endl;
}

void onConnected(TcpSocket *s)
{
	cout<<"connected"<<endl;
}

int main()
{
	TcpSocket tcpSocket;
	tcpSocket.setReadCallBack(onRead);
	tcpSocket.setDisconnectedCallBack(onDisconnected);
	tcpSocket.setConnectedCallBack(onConnected);
	tcpSocket.connectToHost("127.0.0.1", 8000);
	String str;
	while(1)
	{
		cin>>str;
		if(str == "end")
		{
			tcpSocket.abort();
			break;
		}
		str.replace("\\n", "\n").replace("&nbsp;", " ");
		tcpSocket.write(ByteArray::fromString(str, CODEC_UTF8));
	}
	return 0;
}
