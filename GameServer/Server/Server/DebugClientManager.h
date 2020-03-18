#ifndef __DEBUG_MAIN_MANAGER_H__
#define __DEBUG_MAIN_MANAGER_H__

#include <vector>
#include <Windows.h>
#include "Packet.h"

class User;
class Player;
class Matching;
enum DEBUG_PROTOCOL;

class DebugClientManager
{
	struct SendData {
		User* user;
		SendPacket* packet;
	};
private:
	static DebugClientManager* Instance;

	std::vector<SendData> sendList;
	CRITICAL_SECTION cs;

	DebugClientManager() { InitializeCriticalSection(&cs); }
	~DebugClientManager() { DeleteCriticalSection(&cs); }

	void recvUserList(User& user);
	void recvMatchingList(User& user);
	void recvEnterMatching(User& user);
	void recvLeaveMatching(User& user);

	bool isEnteredMatching(User& user, int matching_serial);

public:
	static DebugClientManager* CreateInstance();
	static DebugClientManager* GetInstance();
	static void DestroyInstance();

	void recv(User* user);
	void disconnect(User* user);

	void setSendData(User* user, SendPacket* packet);
	void sendAllSendData();

	void sendConnectUser(User* user);
	void sendDisconnectUser(User* user);
	void sendCreateMatching(Matching* matching);
	void sendDestroyMatching(Matching* matching);
	void sendMatchingFrame(Matching* matching);
	void sendInfoObstacle(User& user, Matching* matching);
	void sendChangeState(Matching* matching, bool isPlayer1);
	void sendEvent(Matching* matching, Player* player, DEBUG_PROTOCOL p, char* event);
	void sendHitAttack(Matching* matching, bool hit, bool attack, Player* enemy);
	void sendPing(Matching* matching, Player* player);
	void sendInfoGimmick(User& user, Matching* matching);
	void sendChangeTemple(Matching* matching);
};

#endif