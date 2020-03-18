#ifndef IOCP_CLIENT_H
#define IOCP_CLIENT_H

#include "TCPClient.h"
#include <string>

class IOCPServer;
enum IOCP_ERROR
{
	IOCPERROR_NONE,		// 오류 없음
	TOTALSIZE_MINUS,	// 패킷의 헤드(totalSize)가 마이너스 값
	TOTALSIZE_TOOBIG,	// 패킷의 헤드(totalSize)가 너무 높음.
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

	WSABUF wsaSendBuf;		// send용 WSA Buf
	WSABUF wsaRecvBuf;		// recv용 WSA Buf
	OVERLAPPED_EX sendOverlapped;
	OVERLAPPED_EX recvOverlapped;
	CRITICAL_SECTION iocp_cs;		// IOCP Server에서 사용		
	IOCPServer* server;				// accept하면 설정됨. 이후 메모리 반환까지 평생 바뀌지 않음

	char* sendBuf;
	int totalSendSize;				// send해야할 용량
	LargePackets largePackets;		// sendBuf용량보다 큰 패킷 저장

	int resultSize;					// recv받은 용량
	bool connected;					// 연결 상태. 한번 끊기면 false. 더 이상 쓸모가 없어짐
	RecvPacket recvPacket;

	bool sendMsgOfAll();
	bool recvMsg();				// 패킷 수신. IOCP는 connect이후 자동으로 recv하고, recvProcess함수로 받게 함
	

public:
	virtual void recvProcess() {};						// recv 완료 시 처리할 함수
	virtual void acceptProcess() {};					// accept 완료 시 처리할 함수
	virtual void disconnectProcess() {};				// 메모리 반환이 가장 안정적. 클라 연결 끊길 시 처리할 함수(정상 종료, 네트워크 장애.  에러 프로세스나 stop() 직접 사용시 호출X)
	virtual void errorProcess(IOCP_ERROR error_bit) {};	// 에러 처리 함수. close 된 후 함수가 실행된다. 뒷 처리는 알아서.


	IOCPClient();
	void stop();
	bool sendMsg(SendPacket& packet);
	bool sendMsg(SendPackets packets);	// 여러 패킷 send.  sendMsg(Packet + Packet + ...)

	bool isConnected();

	int getTotalSendSize();			// send해야할 용량
	int getTotalRecvSize();			// recv받은 전체 용량
	int getPackingRecvSize();		// recv받은 패킹된 용량(헤드 제외)
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


	/* delete 관련*/
	// IOCP Server 처리 도중 외부에서 delete처리 시 안전하게 제거하기 위해.
public:
	~IOCPClient();
	void operator delete(void* _Block) noexcept;
};

#endif