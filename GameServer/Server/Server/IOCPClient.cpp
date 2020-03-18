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

	// ���ۺ��� ū ��Ŷ�� ������ �ִ� ���̳�, ū ��Ŷ send�� �õ��� ��
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

			// ����
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

	// �̹� ������ ���� ��Ŷ�� ���� ��
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

	// ����
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

	// ���ۺ��� ū ��Ŷ�� ������ �ִ� ���̳�, ū ��Ŷ send�� �õ��� ��
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

			// ����
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

	// �̹� ������ ���� ��Ŷ�� ���� ��
	if (totalSendSize > 0)
	{
		packets.memcpyWithEncrypt(sendBuf + totalSendSize);
		totalSendSize += packets.getTotalSize();
		LeaveCriticalSection(&iocp_cs);
		return true;
	}

	packets.memcpyWithEncrypt(sendBuf + totalSendSize);
	totalSendSize += packets.getTotalSize();

	// ����
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
// ��Ŷ ����Ʈ�� �ִ� ��Ŷ�� send()
bool IOCPClient::sendMsgOfAll()
{
	if (totalSendSize == 0)
	{
		return true;
	}

	// ����
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
	// Recv ������ resultSize�� �޾ƾ��� ũ��

	int retval;

	if (resultSize == 0)// ���� ���� �� or �� �������� �� ����
	{
		resultSize = -5;
		recvPacket.initBuffer();
	}
	if (resultSize < 0)// ��ü ũ�� �ް� �ִ� ��
	{
		// ����
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

	if (resultSize > 0)// ���� �����͸� �޾ƿ�
	{
		// ����
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