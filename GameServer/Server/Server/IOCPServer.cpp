#include "IOCPServer.h"
#include "NetworkException.h"

// IOCP 프로세스
DWORD WINAPI IOCPServer::WorkerThread(LPVOID arg) {
	int retval;
	ThreadData* data = (ThreadData*)arg;
	HANDLE hcp = data->hcp;
	IOCPServer* sv = data->server;

	while (1)
	{
		// 비동기 입출력 완료 기다리기
		DWORD cbTransferred;
		PULONG_PTR ulong;
		WSAOVERLAPPED *ptr;
		retval = GetQueuedCompletionStatus(hcp, &cbTransferred,
			(PULONG_PTR)&ulong, (LPOVERLAPPED *)&ptr, INFINITE);

		OVERLAPPED_EX* overlapped_ex = (OVERLAPPED_EX*)ptr;
		IOCPClient *client = (IOCPClient*)overlapped_ex->data;

		EnterCriticalSection(&(client->iocp_cs));

		{
			// 비동기 입출력 결과 확인
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
				if (client->resultSize < 0)// 전체 크기를 받는 중
				{
					// resultSize에 받은 크기만큼 더함
					client->resultSize += cbTransferred;

					// 다 받아졌으면 resultSize를 받아야할 실제 데이터 크기로 변경
					if (client->resultSize == -1)
					{
						// 실제 크기가 마이너스 값인 패킷이 받아졌을 때(해킹 의심)
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
					if (!client->recvMsg())// 남은 데이터도 받음
						PostQueuedCompletionStatus(hcp, 0, (ULONG_PTR)client, &(client->recvOverlapped.overlapped));
					LeaveCriticalSection(&(client->iocp_cs));
					continue;
				}

				if (client->resultSize > 0)// 실제 데이터 받는 중
				{
					// resultSize에 받은 크기만큼 뺌
					client->resultSize -= cbTransferred;

					// 다 받아졌으면 recvBuf 초기화
					if (client->resultSize == 0)
					{
						client->recvPacket.initPointer();
					}
					else {
						if (!client->recvMsg())// 남은 데이터 받음
							PostQueuedCompletionStatus(hcp, 0, (ULONG_PTR)client, &(client->recvOverlapped.overlapped));
						LeaveCriticalSection(&(client->iocp_cs));
						continue;
					}
				}

				// 등록된 recvProcess 함수가 있으면 처리
				EncryptManager::GetInstance()->decoding(client->recvPacket.getPackingBuffer(), client->recvPacket.getPackingSize());

				LeaveCriticalSection(&(client->iocp_cs));

				client->recvProcess();

				if (!client->recvMsg())// 새로운 패킷을 recv함
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
				// 항시 recv 상태를 유지
				if (!client->recvMsg())
					PostQueuedCompletionStatus(hcp, 0, (ULONG_PTR)client, &(client->recvOverlapped.overlapped));

				// 동적할당으로 생성된 accept용 OVERLAPPED_EX는 메모리 해제
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
// 서버 열기
bool IOCPServer::start(int port, const char* ip) {
	// 주소 설정
	SOCKADDR_IN address;
	ZeroMemory(&address, sizeof(address));
	address.sin_family = AF_INET;
	if (ip == 0)
		address.sin_addr.s_addr = htonl(INADDR_ANY); // IP
	else
		address.sin_addr.s_addr = inet_addr(ip);	// IP
	address.sin_port = htons(port);			// 연결할 포트번호

	int retval;

	if (sock != 0)	return true;

	// 윈속 초기화
	if (SocketInit::start() == false)
	{
		return false;
	}

	// 입출력 완료 포트 생성
	hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (hcp == NULL) return false;

	// CPU 개수 확인
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	// (CPU 개수 * 2)개의 작업자 스레드 생성
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

	// 소켓 생성
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) return false;

	// bind
	retval = bind(sock, (SOCKADDR *)&address, sizeof(address));// 개통!
	if (retval == SOCKET_ERROR) return false;

	// listen
	retval = listen(sock, SOMAXCONN);		// 일반 소켓을 Listen 소켓으로 만듦. 오직 연결 요청만 받는 특별한 소켓이 됨
	if (retval == SOCKET_ERROR) return false;

	this->sock = sock;
	connected = true;
	return true;
}
// 클라 받기
bool IOCPServer::acceptClient(IOCPClient* client)
{
	if (client->server != nullptr)
	{
		throw new NewtorkException("한 번 accept받은 객체는 다시 accept하지 못함.", FAIL_ACCEPT);
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

	// IOCP CLient 객체 생성
	client->sock = client_sock;
	client->sendBuf = new char[sendSize];
	client->connected = true;
	client->server = this;

	DWORD recved = 0;
	OVERLAPPED_EX *overlapped_ex = new OVERLAPPED_EX((void*)client, OVERLAPPEDTYPE::OLT_ACCEPT);
	// 입출력 포트 연결
	CreateIoCompletionPort((HANDLE)client_sock, hcp, (ULONG_PTR)client, 0);

	PostQueuedCompletionStatus(hcp, 0, (ULONG_PTR)client, &(overlapped_ex->overlapped));

	return true;
}
// 서버 닫기
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

			int delta = (inst.time + 1000) - clock();// 1000ms 뒤 삭제
			if (delta > 0) Sleep(delta);

			DeleteCriticalSection(&inst.client->iocp_cs);
			if (inst.client->sendBuf != nullptr)
				delete inst.client->sendBuf;
			free(inst.client);
		}
	}
}