#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include "SocketGlobal.h"
#include "Packet.h"

// Ŭ���̾�Ʈ
class TCPClient : public Socket
{
public:
	TCPClient();
	~TCPClient();
	bool start(int port, const char* ip=0);			// ���� ���� �� ��� ����
	void stop();			// ���� ��ȯ
	//virtual bool sendMsg();			// ��Ŷ ����

	bool sendMsg(SendPacket& packet);
	bool sendMsg(SendPackets packets);
	bool recvMsg();			// ��Ŷ ����
	bool isConnected();


private:
	bool connected;
	RecvPacket recvPacket;
	int recvn(SOCKET s, char *buf, int len, int flags);
	CRITICAL_SECTION cs;


	// packet
public:
	int getTotalRecvSize();
	int getPackingRecvSize();
	void getData(void* data, int size);
	bool canGetData(int size);
	template<class T>
	Packet& operator>>(T& data) { recvPacket.getData(&data, sizeof(data));	return recvPacket; }
	template<class T>
	Packet& operator>>(T* data) { recvPacket.getData(data, sizeof(*data)); return recvPacket; }
	template<>
	Packet& operator>>(char* data) {
		int size;
		recvPacket.getData(&size, sizeof(size));
		recvPacket.getData(data, size);
		return recvPacket;
	}
	template<>
	Packet& operator>>(wchar_t* data) {
		int size;
		recvPacket.getData(&size, sizeof(size));
		recvPacket.getData(data, size);
		return recvPacket;
	}
	template<>
	Packet& operator>>(std::string& data) {
		int size;
		recvPacket.getData(&size, sizeof(size));
		char* str = new char[size];
		recvPacket.getData(str, size);
		data = str;
		delete str;
		return recvPacket;
	}
};

#endif