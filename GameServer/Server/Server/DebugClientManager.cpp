#include "User.h"
#include "DebugClientManager.h"
#include "DataBase.h"
#include "GlobalProtocol.h"
#include "Matching.h"
#include "Global.h"
#include "DebugClientState.h"
#include "StaticGameData.h"
#include <vector>

DebugClientManager* DebugClientManager::Instance = nullptr;

DebugClientManager* DebugClientManager::CreateInstance()
{
	if (Instance == nullptr)
	{
		Instance = new DebugClientManager();
	}
	return Instance;
}
DebugClientManager* DebugClientManager::GetInstance()
{
	return Instance;
}
void DebugClientManager::DestroyInstance()
{
	if (Instance != nullptr)
	{
		delete Instance;
		Instance = nullptr;
	}
}

void DebugClientManager::recv(User* user)
{
	DEBUG_PROTOCOL protocol;
	*user >> protocol;

	switch (protocol)
	{
	case DEBUG_PROTOCOL::CLIENT_USERLIST:
		recvUserList(*user);
		break;
	case DEBUG_PROTOCOL::CLIENT_MATCHINGLIST:
		recvMatchingList(*user);
		break;
	case DEBUG_PROTOCOL::CLIENT_ENTER_MATCHING2:
		recvEnterMatching(*user);
		break;
	case DEBUG_PROTOCOL::CLIENT_LEAVE_MATCHING:
		recvLeaveMatching(*user);
		break;
	}
}
void DebugClientManager::setSendData(User* user, SendPacket* packet)
{
	EnterCriticalSection(&cs);

	SendData data;
	data.packet = packet;
	data.user = user;
	sendList.push_back(data);

	LeaveCriticalSection(&cs);
}
void DebugClientManager::sendAllSendData()
{
	EnterCriticalSection(&cs);
	auto list = sendList;
	sendList.clear();
	LeaveCriticalSection(&cs);

	while (true) {
		if (list.size() <= 0) {
			break;
		}
		list[0].user->sendMsg(*(list[0].packet));
		delete list[0].packet;
		list.erase(list.begin());
	}

	/*EnterCriticalSection(&cs);

	while (true) {
		if (sendList.size() <= 0) {
			break;
		}
		sendList[0].user->sendMsg(*(sendList[0].packet));
		delete sendList[0].packet;
		sendList.erase(sendList.begin());
	}

	LeaveCriticalSection(&cs);*/
}
void DebugClientManager::disconnect(User* user)
{
	DataBaseManager::GetInstance()->removeDebugUser(user);
}
void DebugClientManager::recvUserList(User& user)
{
	DataBaseManager* manager = DataBaseManager::GetInstance();
	SendPacket packet(100);

	manager->EnterCS();

	int count = manager->getUserCount();
	char ip[16];

	packet << DEBUG_PROTOCOL::SERVER_USERLIST << user.getSerial() << count;
	for (int i = 0; i < count; i++)
	{
		User* _user = manager->getUser(i);
		_user->getIP(ip);
		packet << ip << _user->getSerial() << _user->getStateType();
	}
	manager->LeaveCS();

	user.sendMsg(packet);
}

void DebugClientManager::sendConnectUser(User* user)
{
	DataBaseManager* manager = DataBaseManager::GetInstance();
	manager->EnterCS();

	int count = manager->getDebugUserCount();
	char ip[16];
	user->getIP(ip);

	for (int i = 0; i < count; i++)
	{
		User& _user = *(manager->getDebugUser(i));

		SendPacket* packet = new SendPacket(50);

		(*packet) << DEBUG_PROTOCOL::SERVER_CONNECT_USER;
		(*packet) << ip << user->getSerial() << user->getStateType();
		setSendData(&_user, packet);
	}

	manager->LeaveCS();

	sendAllSendData();
}
void DebugClientManager::sendDisconnectUser(User* user)
{
	DataBaseManager* manager = DataBaseManager::GetInstance();
	manager->EnterCS();

	int count = manager->getDebugUserCount();
	for (int i = 0; i < count; i++)
	{
		User& _user = *(manager->getDebugUser(i));
		SendPacket* packet = new SendPacket(10);

		(*packet) << DEBUG_PROTOCOL::SERVER_DISCONNECT_USER;
		(*packet) << user->getSerial();
		setSendData(&_user, packet);
	}

	manager->LeaveCS();

	sendAllSendData();
}
void DebugClientManager::recvMatchingList(User& user)
{
	DataBaseManager* manager = DataBaseManager::GetInstance();
	manager->EnterCS();

	int count = manager->getMatchingCount();
	int serial1, serial2;

	SendPacket packet(100);
	packet << DEBUG_PROTOCOL::SERVER_MATCHINGLIST << count;

	for (int i = 0; i < count; i++)
	{
		Matching* m = manager->getMatching(i);
		serial1 = m->getPlayer1()->userInfo->getSerial();
		serial2 = m->getPlayer2()->userInfo->getSerial();

		packet << m->getSerial() << serial1 << serial2;
	}
	manager->LeaveCS();

	user.sendMsg(packet);
}
void DebugClientManager::sendCreateMatching(Matching* matching)
{
	DataBaseManager* manager = DataBaseManager::GetInstance();
	manager->EnterCS();

	int count = manager->getDebugUserCount();
	for (int i = 0; i < count; i++)
	{
		User& _user = *(manager->getDebugUser(i));
		SendPacket* packet = new SendPacket(20);

		(*packet) << DEBUG_PROTOCOL::SERVER_CREATE_MATCHING;
		(*packet) << matching->getSerial();
		(*packet) << matching->getPlayer1()->userInfo->getSerial();
		(*packet) << matching->getPlayer2()->userInfo->getSerial();
		setSendData(&_user, packet);
	}

	manager->LeaveCS();

	sendAllSendData();
}
void DebugClientManager::sendDestroyMatching(Matching* matching)
{
	DataBaseManager* manager = DataBaseManager::GetInstance();
	manager->EnterCS();

	int count = manager->getDebugUserCount();
	for (int i = 0; i < count; i++)
	{
		User& _user = *(manager->getDebugUser(i));
		SendPacket* packet = new SendPacket(10);

		(*packet) << DEBUG_PROTOCOL::SERVER_DESTROY_MATCHING;
		(*packet) << matching->getSerial();
		setSendData(&_user, packet);
	}

	manager->LeaveCS();

	sendAllSendData();
}
void DebugClientManager::recvEnterMatching(User& user)
{
	DataBaseManager* manager = DataBaseManager::GetInstance();
	manager->EnterCS();

	Matching* enterMatching = nullptr;
	SendPacket packet(1000);
	int serial, count;

	user >> serial;

	count = manager->getMatchingCount();
	for (int i = 0; i < count; i++) {
		Matching* m = manager->getMatching(i);
		MatchingData& matchingData = m->getMatchingData();
		Rect field = matchingData.map_size;
		if (m->getSerial() == serial)
		{
			enterMatching = m;

			packet << DEBUG_PROTOCOL::SERVER_ENTER_MATCHING2 << serial;
			packet << field.leftBottom << field.rightTop << matchingData.crt_radius << matchingData.obstacle_radius;

			Player* player[] = { m->getPlayer1(), m->getPlayer2() };
			for (int j = 0; j < 2; j++) {
				packet << (int)player[j]->getState() << player[j]->joystick;
				packet << player[j]->ability.getSpeed();
				packet << player[j]->crtCount;
				for (int k = 0; k < 4; k++) {
					packet << player[j]->crtList[k].index;
					packet << player[j]->crtList[k].position;
					packet << player[j]->crtList[k].getMaxHP();
					packet << player[j]->crtList[k].hp;
				}
			}
			break;
		}
	}

	manager->LeaveCS();

	if (enterMatching != nullptr) {
		DebugClientState* state = (DebugClientState*)user.getStateObject();
		state->matchingList.push_back(enterMatching->getSerial());

		user.sendMsg(packet);
		sendInfoObstacle(user, enterMatching);
		sendInfoGimmick(user, enterMatching);
		sendChangeTemple(enterMatching);
	}
}
void DebugClientManager::sendMatchingFrame(Matching* matching)
{
	DataBaseManager* manager = DataBaseManager::GetInstance();
	manager->EnterCS();

	int count = manager->getDebugUserCount();
	for (int i = 0; i < count; i++)
	{
		User& user = *(manager->getDebugUser(i));
		if (isEnteredMatching(user, matching->getSerial()) == false)
		{
			continue;
		}

		SendPacket* packet = new SendPacket(200);

		(*packet) << DEBUG_PROTOCOL::SERVER_MATCHING_FRAME;
		(*packet) << matching->getSerial() << matching->getTemple().gauge << matching->getCurrentFrame();
		for (int j = 0; j < 2; j++) {
			Player& player = *(matching->getPlayer(j));
			(*packet) << player.crtList[0].direction;
			(*packet) << player.joystick;
			(*packet) << player.ability.getStamina();
			(*packet) << player.ability.getSkillGauge();
			(*packet) << player.crtCount;
			for (int k = 0; k < player.crtCount; k++) {
				(*packet) << player.crtList[k].position;
			}
		}
		setSendData(&user, packet);
	}

	manager->LeaveCS();

	sendAllSendData();
}
void DebugClientManager::recvLeaveMatching(User& user)
{
	int serial;
	user >> serial;

	std::vector<int>& list = ((DebugClientState*)user.getStateObject())->matchingList;

	int count2 = list.size();
	for (int j = 0; j < count2; j++) {
		if (list[j] == serial) {
			list.erase(list.begin() + j);
			break;
		}
	}
}

void DebugClientManager::sendInfoObstacle(User& user, Matching* matching)
{
	if (isEnteredMatching(user, matching->getSerial()) == false)
	{
		return;
	}

	SendPacket packet(1000);
	MatchingData& matchingData = matching->getMatchingData();

	int count = matchingData.obstacle.size();
	packet << DEBUG_PROTOCOL::SERVER_INFO_OBSTACLE << matching->getSerial() << count;
	for (int j = 0; j < count; j++) {
		packet << matchingData.obstacle[j];
	}

	user.sendMsg(packet);
}
void DebugClientManager::sendChangeState(Matching* matching, bool isPlayer1)
{
	DataBaseManager* manager = DataBaseManager::GetInstance();
	manager->EnterCS();

	int count = manager->getDebugUserCount();
	for (int i = 0; i < count; i++)
	{
		User& user = *(manager->getDebugUser(i));
		if (isEnteredMatching(user, matching->getSerial()) == false)
		{
			continue;
		}

		SendPacket* packet = new SendPacket(30);

		(*packet) << DEBUG_PROTOCOL::SERVER_CHANGE_STATE << matching->getSerial();
		if (isPlayer1) {
			(*packet) << 0 << (int)matching->getPlayer1()->getState();
			(*packet) << matching->getPlayer1()->ability.getSpeed();
		}
		else {
			(*packet) << 1 << (int)matching->getPlayer2()->getState();
			(*packet) << matching->getPlayer2()->ability.getSpeed();
		}
		setSendData(&user, packet);
	}
	manager->LeaveCS();

	sendAllSendData();
}
void DebugClientManager::sendHitAttack(Matching* matching, bool hit, bool attack, Player* enemy)
{
	DataBaseManager* manager = DataBaseManager::GetInstance();
	manager->EnterCS();

	int count = manager->getDebugUserCount();
	for (int i = 0; i < count; i++)
	{
		User& user = *(manager->getDebugUser(i));
		if (isEnteredMatching(user, matching->getSerial()) == false)
		{
			continue;
		}

		SendPacket* packet = new SendPacket(50);

		(*packet) << DEBUG_PROTOCOL::SERVER_HIT_ATTACK << matching->getSerial() << hit << attack;
		(*packet) << (matching->getPlayer1() == enemy ? 1 : 0) << enemy->crtCount;
		for (int i = 0; i < enemy->crtCount; i++) {
			(*packet) << enemy->crtList[i].index;
			(*packet) << enemy->crtList[i].hp;
		}
		setSendData(&user, packet);
	}
	manager->LeaveCS();
	sendAllSendData();
}
//
void DebugClientManager::sendEvent(Matching* matching, Player* player, DEBUG_PROTOCOL p, char* event)
{
	DataBaseManager* manager = DataBaseManager::GetInstance();
	manager->EnterCS();

	int count = manager->getDebugUserCount();
	for (int i = 0; i < count; i++)
	{
		User& user = *(manager->getDebugUser(i));
		if (isEnteredMatching(user, matching->getSerial()) == false)
		{
			continue;
		}
		SendPacket* packet = new SendPacket(200);

		(*packet) << p << matching->getSerial();
		if (matching->getPlayer1() == player) {
			(*packet) << 0 << event;
		}
		else
			(*packet) << 1 << event;

		setSendData(&user, packet);
	}
	manager->LeaveCS();
	sendAllSendData();
}
bool DebugClientManager::isEnteredMatching(User& user, int matching_serial)
{
	std::vector<int>& list = ((DebugClientState*)user.getStateObject())->matchingList;
	int count = list.size();
	bool exist = false;
	for (int j = 0; j < count; j++) {
		if (list[j] == matching_serial) {
			exist = true; break;
		}
	}
	return exist;
}
void DebugClientManager::sendPing(Matching* matching, Player* player)
{
	DataBaseManager* manager = DataBaseManager::GetInstance();
	manager->EnterCS();
	User* _userinfo = player->userInfo;

	int count = manager->getDebugUserCount();
	for (int i = 0; i < count; i++)
	{
		User& user = *(manager->getDebugUser(i));
		if (isEnteredMatching(user, matching->getSerial()) == false)
		{
			continue;
		}

		SendPacket* packet = new SendPacket(20);
		int ping = _userinfo->getPing();

		(*packet) << DEBUG_PROTOCOL::SERVER_DEBUG_PING << matching->getSerial();
		(*packet) << (matching->getPlayer1() == player ? 0 : 1) << ping;
		setSendData(&user, packet);
	}
	manager->LeaveCS();
	sendAllSendData();
}
void DebugClientManager::sendInfoGimmick(User& user, Matching* matching)
{
	if (isEnteredMatching(user, matching->getSerial()) == false)
	{
		return;
	}
	//int count = db_matching.gimmick.size();
	int count = matching->getGimmicksCount();
	SendPacket packet(5000);

	packet << DEBUG_PROTOCOL::SERVER_INFO_GIMMICK << matching->getSerial() <<count;
	for (int i = 0; i < count; i++) {
		packet << matching->getGimmick(i).index;
		matching->getGimmick(i).packing(packet);
	}
	
	user.sendMsg(packet);
}
void DebugClientManager::sendChangeTemple(Matching* matching)
{
	DataBaseManager* manager = DataBaseManager::GetInstance();
	manager->EnterCS();

	int count = manager->getDebugUserCount();
	for (int i = 0; i < count; i++)
	{
		User& user = *(manager->getDebugUser(i));
		if (isEnteredMatching(user, matching->getSerial()) == false)
		{
			continue;
		}

		SendPacket* packet = new SendPacket(100);

		(*packet) << DEBUG_PROTOCOL::SERVER_CHANGE_TEMPLE;
		(*packet) << matching->getSerial();
		matching->getTemple().packing(*packet);

		setSendData(&user, packet);
	}
	manager->LeaveCS();
	sendAllSendData();
}