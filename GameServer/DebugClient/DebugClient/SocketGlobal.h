#ifndef SOCKET_GLOBAL_H
#define SOCKET_GLOBAL_H

#pragma warning (disable:4996)
#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <ws2tcpip.h>

#include "EncryptManager.h"

#define SERVERIP "127.0.0.1"
#define ENCRYPT_KEY	549815485


// 소켓
class Socket {
public:

	SOCKET sock;
	Socket();
	bool setBlocking(bool setBlocking);
	bool setNagle(bool nagle);
	bool setTTL(int ttl);

	int getSendBufferSize();
	bool setSendBufferSize(int size);
	int getRecvBufferSize();
	bool setRecvBufferSize(int size);
	void getIP(char* ip);
};
enum OVERLAPPEDTYPE {
	OLT_ACCEPT,
	OLT_RECV,
	OLT_SEND,
};
struct OVERLAPPED_EX {
	WSAOVERLAPPED	overlapped;
	OVERLAPPEDTYPE	type;
	void*			data;

	OVERLAPPED_EX(
		void* _data,
		OVERLAPPEDTYPE _type)
	{
		data = _data;
		type = _type;
		ZeroMemory(&overlapped, sizeof(WSAOVERLAPPED));
	}
};
// 소켓 사용 전 초기화
class SocketInit
{
public:
	static bool start() {// 소켓 사용 전 호출

		if (initialized)	return true;

		// 윈속 초기화
		WSADATA wsa;
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)	// 소켓 DLL 초기화(DLL을 메모리에 올림). 사용하기에 앞서 "ws2_32"라이브러리를 포함시켜줘야함.( #pragma comment(lib, "ws2_32")  )
			return false;

		initialized = true;

		EncryptManager::CreateInstance(ENCRYPT_KEY);
		return true;
	}
	static void stop() {// 소켓 사용 후 호출

		if (initialized == false)	return;

		// 윈속 종료
		WSACleanup();	// 소켓 DLL 메모리 해제
		initialized = false;

		EncryptManager::DestroyInstance();
	}

private:
	static bool initialized;
};

#endif