#ifndef __NETWORK_MANAGER_H__
#define __NETWORK_MANAGER_H__

#include "Global.h"
#include "TCPClient.h"
#include <vector>

class TCPClient;

struct Client
{
	char ip[16];
	int serial;
	USER_STATE state;
};
struct Matching
{
	int serial;
	int player1;	// 유저 serial 번호
	int player2;	// 유저 serial 번호
};

class NetworkManager
{
private:
	static NetworkManager* Instance;
	TCPClient& client = *(new TCPClient());
	HANDLE thread;
	bool connected;
	int serial;
	std::vector<Client*> clientList;
	std::vector<Matching*> matchingList;

	NetworkManager();
	~NetworkManager();
	void recvLicenceResult();
	void recvUserList();
	void recvMatchingList();
	void recvConnectUser();
	void recvDisconnectUser();
	void recvCreateMatching();
	void recvDestroyMatching();
	void recvEnterMatching();
	void recvMatchingFrame();
	void recvInfoObstacle();
	void recvChangeState();
	void recvEvent(DEBUG_PROTOCOL p);
	void recvHitAttack();
	void recvPing();
	void recvInfoGimmick();
	void recvChangeTemple();

public:
	static NetworkManager* CreateInstance();
	static NetworkManager* GetInstance();
	static void DestroyInstance();

	bool connect();
	bool isConnect();
	int getSerial();

	static DWORD WINAPI RecvProcess(void*);

	int getClientCount();
	int getMatchingCount();
	Client* getClient(int index);
	const char* getIP(int clientSerial);
	Matching* getMatching(int index);
	Matching* getMatchingOf(int serial);

	void sendEnterMatching(int serial);
	void sendLeaveMatching(int serial);
};

#endif