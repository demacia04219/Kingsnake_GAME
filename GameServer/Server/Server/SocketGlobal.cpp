#include "SocketGlobal.h"

// 소켓 초기화 여부 변수 초기화
bool SocketInit::initialized = false;

Socket::Socket() {
	sock = 0;
}
bool Socket::setBlocking(bool setBlocking)
{
	u_long on;
	if (setBlocking)	on = 0;
	else				on = 1;

	int retval = ioctlsocket(sock, FIONBIO, &on);
	if (retval == SOCKET_ERROR) return false;
	return true;
}
bool Socket::setNagle(bool nagle)
{
	int option = (int)(!nagle);               //네이글 알고리즘 (0 = on, 1 = off)
	int retval = setsockopt(sock,          
		IPPROTO_TCP,          
		TCP_NODELAY,          
		(const char*)&option, 
		sizeof(option));
	if (retval == SOCKET_ERROR) return false;
	return true;
}
bool Socket::setTTL(int ttl)
{
	// ttl = 2 => 라우터 1개 통과. (설정 안하면 기본적으로 1. 라우터를 빠져나가지 못함)
	int retval = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL,
		(char *)&ttl, sizeof(ttl));
	if (retval == SOCKET_ERROR) return false;
	return true;
}
int Socket::getSendBufferSize()
{
	int optval, optlen;
	optlen = sizeof(optval);
	int retval = getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *)&optval, &optlen);
	if (retval == SOCKET_ERROR) {
		return -1;
	}
	return optval;
}
bool Socket::setSendBufferSize(int size)
{
	int retval = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *)&size, sizeof(size));
	if (retval == SOCKET_ERROR) {
		return false;
	}
	return true;
}
int Socket::getRecvBufferSize()
{
	int optval, optlen;
	optlen = sizeof(optval);
	int retval = getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&optval, &optlen);
	if (retval == SOCKET_ERROR) {
		return -1;
	}
	return optval;
}
bool Socket::setRecvBufferSize(int size)
{
	int retval = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&size, sizeof(size));
	if (retval == SOCKET_ERROR) {
		return false;
	}
	return true;
}
void Socket::getIP(char* ip)
{
	SOCKADDR_IN addr;
	int addrlen = sizeof(addr);
	getpeername(sock, (SOCKADDR*)&addr, &addrlen);
	//printf("[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
	//inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
	strcpy(ip, inet_ntoa(addr.sin_addr));
}