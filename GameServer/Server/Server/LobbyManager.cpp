#include "User.h"
#include "Player.h"
#include "DebugClientManager.h"
#include "LobbyManager.h"
#include "Matching.h"
#include <time.h>
#include "DBServerManager.h"
#include "DataBase.h"
#include "MatchingManager.h"
#include <iostream>

LobbyManager* LobbyManager::Instance = nullptr;

LobbyManager* LobbyManager::CreateInstance()
{
	if (Instance == nullptr)
	{
		Instance = new LobbyManager();
	}
	return Instance;
}
LobbyManager* LobbyManager::GetInstance()
{
	return Instance;
}
void LobbyManager::DestroyInstance()
{
	if (Instance != nullptr)
	{
		delete Instance;
		Instance = nullptr;
	}
}
void LobbyManager::recv(User* user)
{
	// 프로토콜 추출											
	GAME_PROTOCOL protocol;
	*user >> protocol;

	// 프로토콜 판별
	switch (protocol)
	{
	case GAME_PROTOCOL::CLIENT_PONG:
		recvPong(*user);
		break;
	case GAME_PROTOCOL::CLIENT_SET_SUBCHARACTER:
		recvSetSubCharacter(*user);
		break;
	case GAME_PROTOCOL::CLIENT_SET_ITEMSKILL:
		recvSetItemSkill(*user);
		break;
	case GAME_PROTOCOL::CLIENT_SET_COSTUME:
		recvSetCostume(*user);
		break;
	case GAME_PROTOCOL::CLIENT_REQ_MATCHING:
		recvReqMatching(*user);
		break;
	default:
		break;
	}
}
void LobbyManager::disconnect(User* user)
{

}
void LobbyManager::recvPong(User& user)
{
	user.setPong();
}
void LobbyManager::recvSetSubCharacter(User& user)
{
	SendPacket sp;
	sp << SERVER_SET_SUBCHARACTER;
	if (!user.getIsLogin()) {
		sp << false;
		user.sendMsg(sp);
		return;
	}

	int slotIdx, subIdx;
	user >> slotIdx >> subIdx;

	if (slotIdx < 0 || slotIdx > 2) {
		sp << false;
		user.sendMsg(sp);
		return;
	}

	user.EnterCS();

	user.getUserData().character.subCharacterSlot[slotIdx] = subIdx;

	user.LeaveCS();

	sp << true;
	user.sendMsg(sp);

	DBServerManager::GetInstance()->sendSetCharacterDack(user);
}
void LobbyManager::recvSetItemSkill(User& user)
{
	SendPacket sp;
	sp << SERVER_SET_ITEMSKILL;
	if (!user.getIsLogin()) {
		sp << false;
		user.sendMsg(sp);
		return;
	}

	int slotIdx, itemIdx;
	user >> slotIdx >> itemIdx;

	if (slotIdx < 0 || slotIdx > 2) {
		sp << false;
		user.sendMsg(sp);
		return;
	}

	// 소유한 아이템인가?
	bool exist = false;
	user.EnterCS();
	for (auto d : user.getUserData().item.items) 
		if (d == itemIdx) { exist = true; break; }
	if (!exist) {
		sp << false;
		user.sendMsg(sp);
		user.LeaveCS();
		return;
	}

	user.getUserData().item.itemSlot[slotIdx] = itemIdx;
	user.LeaveCS();

	sp << true;
	user.sendMsg(sp);

	DBServerManager::GetInstance()->sendSetItemDack(user);
}
void LobbyManager::recvSetCostume(User& user)
{
	SendPacket sp;
	sp << SERVER_SET_COSTUME;
	if (!user.getIsLogin()) {
		sp << false;
		user.sendMsg(sp);
		return;
	}

	int subIdx, costumeIdx;
	user >> subIdx >> costumeIdx;

	user.EnterCS();
	UserCharacterData& crt = user.getUserData().character;
	if (subIdx < 0 || subIdx >= crt.configCostume.size()) {
		sp << false;
		user.sendMsg(sp);
		user.LeaveCS();
		return;
	}

	// 소유한 코스튬인가?
	bool exist = false;
	for (auto d : crt.costumes)
		if (d == costumeIdx) { exist = true; break; }
	if (!exist) {
		sp << false;
		user.sendMsg(sp);
		user.LeaveCS();
		return;
	}

	// 해당 캐릭터에 적용 가능한 코스튬인가?
	if (db_costume[costumeIdx].subCharacter != subIdx) {
		sp << false;
		user.sendMsg(sp);
		user.LeaveCS();
		return;
	}

	// 적용
	crt.configCostume[subIdx] = costumeIdx;
	user.LeaveCS();

	sp << true;
	user.sendMsg(sp);

	DBServerManager::GetInstance()->sendSetCharacterInventory(user);
}
void LobbyManager::recvReqMatching(User& user)
{
	MatchingManager::GetInstance()->matching(&user);
}