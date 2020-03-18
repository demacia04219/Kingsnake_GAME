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
		clock_t time;		// addDeleteTarget ����
		IOCPClient* client;
	};

	static DWORD WINAPI WorkerThread(LPVOID arg);	// IOCP ó�� ������

	int threadCount;
	int sendSize;		// send ������ ũ��(= �� ���� ���� �� �ִ� �뷮�� ũ��)
	int allowRecvSize;	// recv�� �� ��Ŷ �뷮 ��뷮. �ʰ��� Ŭ�� ������ ������ ����
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
	~IOCPServer();									// �̱���
	bool start(int port, const char* ip = 0);		// ���� ����
	bool acceptClient(IOCPClient* client);			// Ŭ�� �ޱ�
	void stop();									// ���� �ݱ�
	int getThreadCount();
	bool isConnected();
};

#endif