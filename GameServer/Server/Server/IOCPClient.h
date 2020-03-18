#ifndef IOCP_CLIENT_H
#define IOCP_CLIENT_H

#include "TCPClient.h"
#include <string>

class IOCPServer;
enum IOCP_ERROR
{
	IOCPERROR_NONE,		// ���� ����
	TOTALSIZE_MINUS,	// ��Ŷ�� ���(totalSize)�� ���̳ʽ� ��
	TOTALSIZE_TOOBIG,	// ��Ŷ�� ���(totalSize)�� �ʹ� ����.
};

// IOCP Client
class IOCPClient : public Socket
{
	friend class IOCPServer;

private:
	struct LargePackets {
		LargePackets() :sentSize(0), isSending(false) {}
		SendPackets packets;
		int sentSize;
		bool isSending;
	};

	WSABUF wsaSendBuf;		// send�� WSA Buf
	WSABUF wsaRecvBuf;		// recv�� WSA Buf
	OVERLAPPED_EX sendOverlapped;
	OVERLAPPED_EX recvOverlapped;
	CRITICAL_SECTION iocp_cs;		// IOCP Server���� ���		
	IOCPServer* server;				// accept�ϸ� ������. ���� �޸� ��ȯ���� ��� �ٲ��� ����

	char* sendBuf;
	int totalSendSize;				// send�ؾ��� �뷮
	LargePackets largePackets;		// sendBuf�뷮���� ū ��Ŷ ����

	int resultSize;					// recv���� �뷮
	bool connected;					// ���� ����. �ѹ� ����� false. �� �̻� ���� ������
	RecvPacket recvPacket;

	bool sendMsgOfAll();
	bool recvMsg();				// ��Ŷ ����. IOCP�� connect���� �ڵ����� recv�ϰ�, recvProcess�Լ��� �ް� ��
	

public:
	virtual void recvProcess() {};						// recv �Ϸ� �� ó���� �Լ�
	virtual void acceptProcess() {};					// accept �Ϸ� �� ó���� �Լ�
	virtual void disconnectProcess() {};				// �޸� ��ȯ�� ���� ������. Ŭ�� ���� ���� �� ó���� �Լ�(���� ����, ��Ʈ��ũ ���.  ���� ���μ����� stop() ���� ���� ȣ��X)
	virtual void errorProcess(IOCP_ERROR error_bit) {};	// ���� ó�� �Լ�. close �� �� �Լ��� ����ȴ�. �� ó���� �˾Ƽ�.


	IOCPClient();
	void stop();
	bool sendMsg(SendPacket& packet);
	bool sendMsg(SendPackets packets);	// ���� ��Ŷ send.  sendMsg(Packet + Packet + ...)

	bool isConnected();

	int getTotalSendSize();			// send�ؾ��� �뷮
	int getTotalRecvSize();			// recv���� ��ü �뷮
	int getPackingRecvSize();		// recv���� ��ŷ�� �뷮(��� ����)
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


	/* delete ����*/
	// IOCP Server ó�� ���� �ܺο��� deleteó�� �� �����ϰ� �����ϱ� ����.
public:
	~IOCPClient();
	void operator delete(void* _Block) noexcept;
};

#endif