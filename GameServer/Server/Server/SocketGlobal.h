#ifndef SOCKET_GLOBAL_H
#define SOCKET_GLOBAL_H

#pragma warning (disable:4996)
#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <ws2tcpip.h>

#include "EncryptManager.h"

#define SERVERIP "127.0.0.1"
#define ENCRYPT_KEY	549815485


// ����
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
// ���� ��� �� �ʱ�ȭ
class SocketInit
{
public:
	static bool start() {// ���� ��� �� ȣ��

		if (initialized)	return true;

		// ���� �ʱ�ȭ
		WSADATA wsa;
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)	// ���� DLL �ʱ�ȭ(DLL�� �޸𸮿� �ø�). ����ϱ⿡ �ռ� "ws2_32"���̺귯���� ���Խ��������.( #pragma comment(lib, "ws2_32")  )
			return false;

		initialized = true;

		EncryptManager::CreateInstance(ENCRYPT_KEY);
		return true;
	}
	static void stop() {// ���� ��� �� ȣ��

		if (initialized == false)	return;

		// ���� ����
		WSACleanup();	// ���� DLL �޸� ����
		initialized = false;

		EncryptManager::DestroyInstance();
	}

private:
	static bool initialized;
};

#endif