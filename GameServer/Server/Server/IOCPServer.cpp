#include "IOCPServer.h"
#include "NetworkException.h"

// IOCP ���μ���
DWORD WINAPI IOCPServer::WorkerThread(LPVOID arg) {
	int retval;
	ThreadData* data = (ThreadData*)arg;
	HANDLE hcp = data->hcp;
	IOCPServer* sv = data->server;

	while (1)
	{
		// �񵿱� ����� �Ϸ� ��ٸ���
		DWORD cbTransferred;
		PULONG_PTR ulong;
		WSAOVERLAPPED *ptr;
		retval = GetQueuedCompletionStatus(hcp, &cbTransferred,
			(PULONG_PTR)&ulong, (LPOVERLAPPED *)&ptr, INFINITE);

		OVERLAPPED_EX* overlapped_ex = (OVERLAPPED_EX*)ptr;
		IOCPClient *client = (IOCPClient*)overlapped_ex->data;

		EnterCriticalSection(&(client->iocp_cs));

		{
			// �񵿱� ����� ��� Ȯ��
			if (retval == 0 || cbTransferred == 0) {
				if (overlapped_ex->type != OVERLAPPEDTYPE::OLT_ACCEPT
					&& client->connected)
				{
					client->connected = false;
					LeaveCriticalSection(&(client->iocp_cs));

					client->disconnectProcess();
					continue;
				}
			}


			if (overlapped_ex->type == OVERLAPPEDTYPE::OLT_RECV)
			{
				if (client->resultSize < 0)// ��ü ũ�⸦ �޴� ��
				{
					// resultSize�� ���� ũ�⸸ŭ ����
					client->resultSize += cbTransferred;

					// �� �޾������� resultSize�� �޾ƾ��� ���� ������ ũ��� ����
					if (client->resultSize == -1)
					{
						// ���� ũ�Ⱑ ���̳ʽ� ���� ��Ŷ�� �޾����� ��(��ŷ �ǽ�)
						if (client->recvPacket.getPackingSize() < 0) {
							client->stop();
							client->errorProcess(IOCP_ERROR::TOTALSIZE_MINUS);
							LeaveCriticalSection(&(client->iocp_cs));
							continue;
						}
						else if (client->recvPacket.getPackingSize() > sv->allowRecvSize) {
							client->stop();
							client->errorProcess(IOCP_ERROR::TOTALSIZE_TOOBIG);
							LeaveCriticalSection(&(client->iocp_cs));
							continue;
						}
						if (client->recvPacket.getBufferSize() < client->recvPacket.getPackingSize())
						{
							client->recvPacket.setBufferSize(client->recvPacket.getPackingSize());
						}
						client->resultSize = client->recvPacket.getPackingSize();
					}
					if (!client->recvMsg())// ���� �����͵� ����
						PostQueuedCompletionStatus(hcp, 0, (ULONG_PTR)client, &(client->recvOverlapped.overlapped));
					LeaveCriticalSection(&(client->iocp_cs));
					continue;
				}

				if (client->resultSize > 0)// ���� ������ �޴� ��
				{
					// resultSize�� ���� ũ�⸸ŭ ��
					client->resultSize -= cbTransferred;

					// �� �޾������� recvBuf �ʱ�ȭ
					if (client->resultSize == 0)
					{
						client->recvPacket.initPointer();
					}
					else {
						if (!client->recvMsg())// ���� ������ ����
							PostQueuedCompletionStatus(hcp, 0, (ULONG_PTR)client, &(client->recvOverlapped.overlapped));
						LeaveCriticalSection(&(client->iocp_cs));
						continue;
					}
				}

				// ��ϵ� recvProcess �Լ��� ������ ó��
				EncryptManager::GetInstance()->decoding(client->recvPacket.getPackingBuffer(), client->recvPacket.getPackingSize());

				LeaveCriticalSection(&(client->iocp_cs));

				client->recvProcess();

				if (!client->recvMsg())// ���ο� ��Ŷ�� recv��
					PostQueuedCompletionStatus(hcp, 0, (ULONG_PTR)client, &(client->recvOverlapped.overlapped));

				continue;
			}
			else if (overlapped_ex->type == OVERLAPPEDTYPE::OLT_SEND)
			{
				client->totalSendSize -= cbTransferred;

				if (client->largePackets.isSending) {
					memcpy(client->sendBuf, client->sendBuf + cbTransferred, client->totalSendSize);
					client->largePackets.sentSize += cbTransferred;
					int size = client->largePackets.packets.memcpyWithEncrypt(
						client->sendBuf + client->totalSendSize, 
						client->largePackets.sentSize, 
						client->server->sendSize - client->totalSendSize);
					client->totalSendSize += size;

					if (client->totalSendSize == 0) {
						client->largePackets.isSending = false;
						client->largePackets.packets.init(true);
						LeaveCriticalSection(&(client->iocp_cs));
						continue;
					}
					else {
						client->sendMsgOfAll();
					}
				}
				else {
					if (client->totalSendSize > 0)
					{
						memcpy(client->sendBuf, client->sendBuf + cbTransferred, client->totalSendSize);
						client->sendMsgOfAll();
					}
					else {
						if (client->largePackets.packets.getTotalSize() > 0) {
							client->largePackets.isSending = true;
							client->largePackets.sentSize = 0;
							int size = client->largePackets.packets.memcpyWithEncrypt(client->sendBuf, 0, client->server->sendSize);
							client->totalSendSize = size;
							client->sendMsgOfAll();
						}
						else {
							LeaveCriticalSection(&(client->iocp_cs));
							continue;
						}
					}
				}
			}
			else if (overlapped_ex->type == OVERLAPPEDTYPE::OLT_ACCEPT)
			{
				LeaveCriticalSection(&(client->iocp_cs));

				client->acceptProcess();
				// �׽� recv ���¸� ����
				if (!client->recvMsg())
					PostQueuedCompletionStatus(hcp, 0, (ULONG_PTR)client, &(client->recvOverlapped.overlapped));

				// �����Ҵ����� ������ accept�� OVERLAPPED_EX�� �޸� ����
				delete overlapped_ex;

				continue;
			}
		}

		LeaveCriticalSection(&(client->iocp_cs));
	}
}

IOCPServer::IOCPServer(int sendSize, int allowRecvSize) {
	threadCount = 0;
	connected = false;
	this->sendSize = sendSize;
	this->allowRecvSize = allowRecvSize;
	InitializeCriticalSection(&iocp_server_cs);
	deleteEvent = CreateEvent(0, false, false, 0);
}
IOCPServer::~IOCPServer() {
	CloseHandle(deleteEvent);
	DeleteCriticalSection(&iocp_server_cs);
	stop();
}
// ���� ����
bool IOCPServer::start(int port, const char* ip) {
	// �ּ� ����
	SOCKADDR_IN address;
	ZeroMemory(&address, sizeof(address));
	address.sin_family = AF_INET;
	if (ip == 0)
		address.sin_addr.s_addr = htonl(INADDR_ANY); // IP
	else
		address.sin_addr.s_addr = inet_addr(ip);	// IP
	address.sin_port = htons(port);			// ������ ��Ʈ��ȣ

	int retval;

	if (sock != 0)	return true;

	// ���� �ʱ�ȭ
	if (SocketInit::start() == false)
	{
		return false;
	}

	// ����� �Ϸ� ��Ʈ ����
	hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (hcp == NULL) return false;

	// CPU ���� Ȯ��
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	// (CPU ���� * 2)���� �۾��� ������ ����
	HANDLE hThread;
	threadCount = si.dwNumberOfProcessors * 2;
	for (int i = 0; i < (int)si.dwNumberOfProcessors * 2; i++) {
		ThreadData* dt = new ThreadData();
		dt->hcp = hcp;
		dt->server = this;
		hThread = CreateThread(NULL, 0, WorkerThread, dt, 0, NULL);
		if (hThread == NULL) return false;
		CloseHandle(hThread);
	}
	hThread = CreateThread(0, 0, DeleteProcess, this, 0, 0);
	if (hThread == NULL) return false;
	CloseHandle(hThread);
	threadCount++;

	// ���� ����
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) return false;

	// bind
	retval = bind(sock, (SOCKADDR *)&address, sizeof(address));// ����!
	if (retval == SOCKET_ERROR) return false;

	// listen
	retval = listen(sock, SOMAXCONN);		// �Ϲ� ������ Listen �������� ����. ���� ���� ��û�� �޴� Ư���� ������ ��
	if (retval == SOCKET_ERROR) return false;

	this->sock = sock;
	connected = true;
	return true;
}
// Ŭ�� �ޱ�
bool IOCPServer::acceptClient(IOCPClient* client)
{
	if (client->server != nullptr)
	{
		throw new NewtorkException("�� �� accept���� ��ü�� �ٽ� accept���� ����.", FAIL_ACCEPT);
		return false;
	}

	// accept
	SOCKADDR_IN addr;
	int len = sizeof(addr);
	getpeername(sock, (SOCKADDR*)&addr, &len);
	SOCKET client_sock = accept(sock, (SOCKADDR*)&addr, &len);

	if (client_sock == INVALID_SOCKET) {
		sock = 0;
		connected = false;
		return false;
	}

	// IOCP CLient ��ü ����
	client->sock = client_sock;
	client->sendBuf = new char[sendSize];
	client->connected = true;
	client->server = this;

	DWORD recved = 0;
	OVERLAPPED_EX *overlapped_ex = new OVERLAPPED_EX((void*)client, OVERLAPPEDTYPE::OLT_ACCEPT);
	// ����� ��Ʈ ����
	CreateIoCompletionPort((HANDLE)client_sock, hcp, (ULONG_PTR)client, 0);

	PostQueuedCompletionStatus(hcp, 0, (ULONG_PTR)client, &(overlapped_ex->overlapped));

	return true;
}
// ���� �ݱ�
void IOCPServer::stop()
{
	if (sock == 0)	return;

	closesocket(sock);
	sock = 0;
	connected = false;
}
int IOCPServer::getThreadCount()
{
	return threadCount;
}
bool IOCPServer::isConnected()
{
	return connected;
}
void IOCPServer::addDeleteTarget(IOCPClient* client)
{
	EnterCriticalSection(&iocp_server_cs);

	DeleteInstance inst;
	inst.client = client;
	inst.time = clock();

	deleteTarget.push(inst);

	LeaveCriticalSection(&iocp_server_cs);

	SetEvent(deleteEvent);
}
DWORD WINAPI IOCPServer::DeleteProcess(void* arg)
{
	IOCPServer* server = (IOCPServer*)arg;
	while (true)
	{
		WaitForSingleObject(server->deleteEvent, INFINITE);


		while (true)
		{
			DeleteInstance inst;
			{
				EnterCriticalSection(&(server->iocp_server_cs));

				if (server->deleteTarget.size() == 0) {
					LeaveCriticalSection(&(server->iocp_server_cs));
					break;
				}

				inst = server->deleteTarget.front();
				server->deleteTarget.pop();

				LeaveCriticalSection(&(server->iocp_server_cs));
			}

			int delta = (inst.time + 1000) - clock();// 1000ms �� ����
			if (delta > 0) Sleep(delta);

			DeleteCriticalSection(&inst.client->iocp_cs);
			if (inst.client->sendBuf != nullptr)
				delete inst.client->sendBuf;
			free(inst.client);
		}
	}
}