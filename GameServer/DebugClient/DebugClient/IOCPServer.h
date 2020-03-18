#ifndef IOCP_SERVER_H
#define IOCP_SERVER_H

#include "SocketGlobal.h"
#include "IOCPClient.h"
#include <time.h>
#include <queue>

// IOCP Server
class IOCPServer : public Socket
{
	friend class IOCPClient;
private:
	struct ThreadData {
		HANDLE hcp;
		IOCPServer* server;
	};
	struct DeleteInstance {
		clock_t time;		// addDeleteTarget 시점
		IOCPClient* client;
	};

	static DWORD WINAPI WorkerThread(LPVOID arg);	// IOCP 처리 쓰레드

	int threadCount;
	int sendSize;		// send 버퍼의 크기(= 한 번에 보낼 수 있는 용량의 크기)
	int allowRecvSize;	// recv할 때 패킷 용량 허용량. 초과시 클라 연결을 강제로 끊음
	bool connected;

	/*delete manage*/
	CRITICAL_SECTION iocp_server_cs;
	HANDLE deleteEvent;
	std::queue<DeleteInstance> deleteTarget;
	void addDeleteTarget(IOCPClient* client);
	static DWORD WINAPI DeleteProcess(void* arg);

public:
	HANDLE hcp;										// HCP
	IOCPServer(int sendSize=8192, int allowRecvSize=8192);
	~IOCPServer();									// 미구현
	bool start(int port, const char* ip = 0);		// 서버 열기
	bool acceptClient(IOCPClient* client);			// 클라 받기
	void stop();									// 서버 닫기
	int getThreadCount();
	bool isConnected();
};

#endif