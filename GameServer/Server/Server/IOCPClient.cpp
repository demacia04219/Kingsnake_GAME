#include "IOCPServer.h"
#include "NetworkException.h"

IOCPClient::IOCPClient() :
		recvOverlapped(OVERLAPPED_EX(this, OVERLAPPEDTYPE::OLT_RECV)),
		sendOverlapped(OVERLAPPED_EX(this, OVERLAPPEDTYPE::OLT_SEND))
{
	totalSendSize = 0;
	resultSize = 0;
	sendBuf = nullptr;
	connected = false;
	server = nullptr;
	InitializeCriticalSection(&iocp_cs);
}
IOCPClient::~IOCPClient()
{
	if (server == nullptr) {
		DeleteCriticalSection(&iocp_cs);
		if (sendBuf != nullptr)
			delete sendBuf;
		return;
	}

	stop();
	
	server->addDeleteTarget(this);
}
// send()
bool IOCPClient::sendMsg(SendPacket& packet)
{
	if (connected == false)
		return false;

	if (packet.getPackingSize() == 0)
		return true;

	EnterCriticalSection(&iocp_cs);

	// 버퍼보다 큰 패킷을 보내고 있는 중이나, 큰 패킷 send를 시도할 때
	if (largePackets.isSending) {
		Packet* p = new Packet(packet);
		largePackets.packets += *p;
		LeaveCriticalSection(&iocp_cs);
		return true;
	} else if (totalSendSize + packet.getTotalSize() > server->sendSize)
	{ 
		Packet* p = new Packet(packet);
		largePackets.packets += *p;
		if (totalSendSize == 0) {
			largePackets.isSending = true;
			largePackets.sentSize = 0;
			int size = largePackets.packets.memcpyWithEncrypt(sendBuf, 0, server->sendSize);
			totalSendSize = size;

			// 세팅
			ZeroMemory(&sendOverlapped.overlapped, sizeof(sendOverlapped.overlapped));
			wsaSendBuf.buf = sendBuf;
			wsaSendBuf.len = totalSendSize;

			// Send
			DWORD sendbytes;
			int retval = WSASend(sock, &wsaSendBuf, 1, &sendbytes, 0, &sendOverlapped.overlapped, NULL);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() != WSA_IO_PENDING) {
					LeaveCriticalSection(&iocp_cs);
					return false;
				}
			}
		}
		LeaveCriticalSection(&iocp_cs);
		return true;
	}

	// 이미 보내는 중인 패킷이 있을 때
	if (totalSendSize > 0)
	{
		memcpy(sendBuf + totalSendSize, packet.getTotalBuffer(), packet.getTotalSize());
		EncryptManager::GetInstance()->encoding(
			sendBuf + totalSendSize + sizeof(int), packet.getPackingSize());
		totalSendSize += packet.getTotalSize();
		LeaveCriticalSection(&iocp_cs);
		return true;
	}

	//EncryptManager::GetInstance()->encoding(sendBuf->buffer + sizeof(int), sendBuf->GetSize());
	memcpy(sendBuf + totalSendSize, packet.getTotalBuffer(), packet.getTotalSize());
	EncryptManager::GetInstance()->encoding(
		sendBuf + totalSendSize + sizeof(int), packet.getPackingSize());
	totalSendSize += packet.getTotalSize();

	// 세팅
	ZeroMemory(&sendOverlapped.overlapped, sizeof(sendOverlapped.overlapped));
	wsaSendBuf.buf = sendBuf;
	wsaSendBuf.len = totalSendSize;

	// Send
	DWORD sendbytes;
	int retval = WSASend(sock, &wsaSendBuf, 1, &sendbytes, 0, &sendOverlapped.overlapped, NULL);
	if (retval == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING) {
			LeaveCriticalSection(&iocp_cs);
			return false;
		}
	}

	LeaveCriticalSection(&iocp_cs);
	return true;
}

bool IOCPClient::sendMsg(SendPackets packets)
{
	if (connected == false)
		return false;

	if (packets.getTotalSize() == 0)
		return true;

	EnterCriticalSection(&iocp_cs);

	// 버퍼보다 큰 패킷을 보내고 있는 중이나, 큰 패킷 send를 시도할 때
	if (largePackets.isSending) {
		for (Packet* pk : packets.packets) {
			Packet* p = new Packet(*pk);
			largePackets.packets += *p;
		}
		LeaveCriticalSection(&iocp_cs);
		return true;
	}
	else if (totalSendSize + packets.getTotalSize() > server->sendSize)
	{
		for (Packet* pk : packets.packets) {
			Packet* p = new Packet(*pk);
			largePackets.packets += *p;
		}
		if (totalSendSize == 0) {
			largePackets.isSending = true;
			largePackets.sentSize = 0;
			int size = largePackets.packets.memcpyWithEncrypt(sendBuf, 0, server->sendSize);
			totalSendSize = size;

			// 세팅
			ZeroMemory(&sendOverlapped.overlapped, sizeof(sendOverlapped.overlapped));
			wsaSendBuf.buf = sendBuf;
			wsaSendBuf.len = totalSendSize;

			// Send
			DWORD sendbytes;
			int retval = WSASend(sock, &wsaSendBuf, 1, &sendbytes, 0, &sendOverlapped.overlapped, NULL);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() != WSA_IO_PENDING) {
					LeaveCriticalSection(&iocp_cs);
					return false;
				}
			}
		}
		LeaveCriticalSection(&iocp_cs);
		return true;
	}

	// 이미 보내는 중인 패킷이 있을 때
	if (totalSendSize > 0)
	{
		packets.memcpyWithEncrypt(sendBuf + totalSendSize);
		totalSendSize += packets.getTotalSize();
		LeaveCriticalSection(&iocp_cs);
		return true;
	}

	packets.memcpyWithEncrypt(sendBuf + totalSendSize);
	totalSendSize += packets.getTotalSize();

	// 세팅
	ZeroMemory(&sendOverlapped.overlapped, sizeof(sendOverlapped.overlapped));
	wsaSendBuf.buf = sendBuf;
	wsaSendBuf.len = totalSendSize;

	// Send
	DWORD sendbytes;
	int retval = WSASend(sock, &wsaSendBuf, 1, &sendbytes, 0, &sendOverlapped.overlapped, NULL);
	if (retval == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING) {
			LeaveCriticalSection(&iocp_cs);
			return false;
		}
	}

	LeaveCriticalSection(&iocp_cs);
	return true;
}
// 패킷 리스트에 있는 패킷을 send()
bool IOCPClient::sendMsgOfAll()
{
	if (totalSendSize == 0)
	{
		return true;
	}

	// 세팅
	ZeroMemory(&sendOverlapped.overlapped, sizeof(sendOverlapped.overlapped));
	wsaSendBuf.buf = sendBuf;
	wsaSendBuf.len = totalSendSize;

	// Send
	DWORD sendbytes;
	int retval = WSASend(sock, &wsaSendBuf, 1, &sendbytes, 0, &sendOverlapped.overlapped, NULL);
	if (retval == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING) {
			return false;
		}
	}
	return true;
}
// recv()
bool IOCPClient::recvMsg()
{
	// Recv 버퍼의 resultSize는 받아야할 크기

	int retval;

	if (resultSize == 0)// 최초 보낼 때 or 다 보내졌을 때 상태
	{
		resultSize = -5;
		recvPacket.initBuffer();
	}
	if (resultSize < 0)// 전체 크기 받고 있는 중
	{
		// 세팅
		ZeroMemory(&recvOverlapped.overlapped, sizeof(recvOverlapped.overlapped));
		wsaRecvBuf.buf = recvPacket.getTotalBuffer() + (5 + resultSize);
		wsaRecvBuf.len = resultSize * (-1) - 1;

		// Recv
		DWORD recvBytes, flags;
		flags = 0;
		retval = WSARecv(sock, &wsaRecvBuf, 1, &recvBytes, &flags, &recvOverlapped.overlapped, NULL);
		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
				return false;
			}
		}
		return true;
	}

	if (resultSize > 0)// 실제 데이터를 받아옴
	{
		// 세팅
		ZeroMemory(&recvOverlapped.overlapped, sizeof(recvOverlapped.overlapped));
		wsaRecvBuf.buf = recvPacket.getPackingBuffer() + (recvPacket.getPackingSize() - resultSize);
		wsaRecvBuf.len = resultSize;

		// Recv
		DWORD recvBytes, flags;
		flags = 0;
		retval = WSARecv(sock, &wsaRecvBuf, 1, &recvBytes, &flags, &recvOverlapped.overlapped, NULL);
		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
				return false;
			}
		}
		return true;
	}
	return true;
}
void IOCPClient::stop()
{
	EnterCriticalSection(&iocp_cs);
	if (sock == 0) {
		LeaveCriticalSection(&iocp_cs);
		return;
	}

	closesocket(sock);
	sock = 0;
	connected = false;
	LeaveCriticalSection(&iocp_cs);
}
int IOCPClient::getTotalSendSize()
{
	return totalSendSize;
}
bool IOCPClient::isConnected()
{
	return connected;
}
void IOCPClient::getData(void* data, int size)
{
	recvPacket.getData(data, size);
}
bool IOCPClient::canGetData(int size)
{
	return recvPacket.canGetData(size);
}
int IOCPClient::getTotalRecvSize()
{
	return recvPacket.getTotalSize();
}
int IOCPClient::getPackingRecvSize()
{
	return recvPacket.getPackingSize(); 
}
void IOCPClient::operator delete(void*  _Block) noexcept
{
	IOCPClient* client = (IOCPClient*)_Block;
	
	if (client->server == nullptr) {
		free(client);
	}
}