#include "TCPClient.h"

// 생성자
TCPClient::TCPClient()
{
	connected = false;
	InitializeCriticalSection(&cs);
}
// 소멸자
TCPClient::~TCPClient()
{
	stop();
	DeleteCriticalSection(&cs);
}

// socket(), connect()
bool TCPClient::start(int port, const char* ip)
{
	int retval;

	if (sock != 0)	return true;

	SocketInit::start();

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) return false;

	// 주소 설정
	SOCKADDR_IN address;
	ZeroMemory(&address, sizeof(address));
	address.sin_family = AF_INET;
	if (ip == 0)
		address.sin_addr.s_addr = inet_addr("127.0.0.1"); // IP
	else
		address.sin_addr.s_addr = inet_addr(ip);	// IP
	address.sin_port = htons(port);			// 연결할 포트번호

	retval = connect(sock, (SOCKADDR *)&address, sizeof(address));// 연결 요청 함수. 연결 요청 패킷을 보냄. 바인딩 안되있을 경우 bind도 처리
	if (retval == SOCKET_ERROR) return false;

	this->sock = sock;
	connected = true;

	return true;
}

// closesocket()
void TCPClient::stop()
{
	EnterCriticalSection(&cs);
	if (sock == 0) {
		LeaveCriticalSection(&cs);
		return;
	}

	closesocket(sock);
	sock = 0;
	connected = false;
	LeaveCriticalSection(&cs);
}
bool TCPClient::sendMsg(SendPacket& packet)
{
	EnterCriticalSection(&cs);

	if (connected == false) {
		LeaveCriticalSection(&cs);
		return false;
	}

	if (packet.getPackingSize() == 0) {
		LeaveCriticalSection(&cs);
		return true;
	}

	packet.initPointer();

	Packet p = packet;
	EncryptManager::GetInstance()->encoding(
		p.getPackingBuffer(), p.getPackingSize());

	int retval;
	int result = 0;
	while (result != p.getTotalSize())
	{
		retval = send(sock, p.getTotalBuffer() + result, p.getTotalSize() - result, 0);
		if (retval == SOCKET_ERROR || retval == 0) {
			sock = 0;
			connected = false;
			LeaveCriticalSection(&cs);
			return false;
		}
		result += retval;
	}
	LeaveCriticalSection(&cs);
	return true;
}
bool TCPClient::sendMsg(SendPackets packets)
{
	EnterCriticalSection(&cs);

	if (connected == false) {
		LeaveCriticalSection(&cs);
		return false;
	}

	if (packets.getTotalSize() == 0) {
		LeaveCriticalSection(&cs);
		return true;
	}

	char* buffer = new char[packets.getTotalSize()];
	packets.memcpyWithEncrypt(buffer);

	int retval;
	int result = 0;
	while (result != packets.getTotalSize())
	{
		retval = send(sock, buffer + result, packets.getTotalSize() - result, 0);
		if (retval == SOCKET_ERROR || retval == 0) {
			sock = 0;
			connected = false;
			delete buffer;
			LeaveCriticalSection(&cs);
			return false;
		}
		result += retval;
	}
	delete buffer;
	LeaveCriticalSection(&cs);
	return true;
}
bool TCPClient::recvMsg()
{
	int retval;
	Packet& packet = recvPacket;
	retval = recvn(sock, packet.getTotalBuffer(), 4, 0);
	if (retval == SOCKET_ERROR || retval == 0) {
		sock = 0;
		connected = false;
		return false;
	}

	if(packet.getBufferSize() < packet.getPackingSize())
		packet.setBufferSize(packet.getPackingSize());

	retval = recvn(sock, packet.getPackingBuffer(), packet.getPackingSize(), 0);
	if (retval == SOCKET_ERROR || retval == 0) {
		sock = 0;
		connected = false;
		return false;
	}

	EncryptManager::GetInstance()->decoding(packet.getPackingBuffer(), packet.getPackingSize());

	packet.initPointer();
	return true;
}

// recvn
int TCPClient::recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			return 0;
		left -= received;
		ptr += received;
	}

	return (len - left);
}
void TCPClient::getData(void* data, int size)
{
	recvPacket.getData(data, size);
}
bool TCPClient::canGetData(int size)
{
	return recvPacket.canGetData(size);
}
int TCPClient::getTotalRecvSize()
{
	return recvPacket.getTotalSize();
}
int TCPClient::getPackingRecvSize()
{
	return recvPacket.getPackingSize();
}
bool TCPClient::isConnected() {
	return connected;
}