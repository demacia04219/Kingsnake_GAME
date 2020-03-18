#include "DBServerManager.h"
#include "TCPClient.h"
#include "Global.h"
#include "GlobalProtocol.h"
#include "DynamicGameData.h"
#include "DataBase.h"
#include <stdio.h>
#include <string>
#include "User.h"
#include "CheckState.h"

DBServerManager* DBServerManager::Instance = nullptr;

DBServerManager::DBServerManager() : client(*(new TCPClient()))
{
	connected = false;
}
DBServerManager::~DBServerManager()
{
	client.stop();
}

DBServerManager* DBServerManager::CreateInstance()
{
	if (Instance == nullptr)
	{
		Instance = new DBServerManager();
	}
	return Instance;
}
DBServerManager* DBServerManager::GetInstance()
{
	return Instance;
}
void DBServerManager::DestroyInstance()
{
	if (Instance != nullptr)
	{
		delete Instance;
		Instance = nullptr;
	}
}
void DBServerManager::connectDBServer()
{
	printf("Connecting to DB Server...\n");
	CreateThread(0, 0, DBServerManager::recvProcess, 0, 0, 0);
}
bool DBServerManager::isConnected()
{
	return connected;
}
TCPClient& DBServerManager::getSocket()
{
	return client;
}
DWORD WINAPI DBServerManager::recvProcess(void* arg)
{
	DBServerManager* manager = DBServerManager::GetInstance();
	TCPClient& client = manager->client;
	DB_PROTOCOL p;

	manager->connected = client.start(DBSERVERPORT, DBSERVERIP);
	if (manager->connected)
	{
		printf("Connected to DB Server successfully\n");
	}

	while (true)
	{
		if (client.recvMsg() == false)
		{
			manager->connected = false;
			printf("Disconnected with DB Server\n");
			printf("Connecting to DB Server...\n");

			client.stop();
			while (client.start(DBSERVERPORT, DBSERVERIP) == false) {// 서버 연결 기다림
				client.stop();
			}
			manager->connected = true;
			printf("Connected to DB Server successfully\n");
			continue;
		}
		client >> p;
		switch (p) {
		case DB_PROTOCOL::DBSERVER_RES_SET_BASICINFO:
			manager->recvSetBasicInfo();
			break;
		case DB_PROTOCOL::DBSERVER_RES_SET_MATCHINFO:
			manager->recvSetMatchInfo();
			break;
		case DB_PROTOCOL::DBSERVER_RES_SET_CHARACTER_DACK:
			manager->recvSetCharacterDack();
			break;
		case DB_PROTOCOL::DBSERVER_RES_SET_ITEM_DACK:
			manager->recvSetItemDack();
			break;
		case DB_PROTOCOL::DBSERVER_RES_SET_EMOTION_DACK:
			manager->recvSetEmotionDack();
			break;
		case DB_PROTOCOL::DBSERVER_RES_SET_CHARACTER_INVENTORY:
			manager->recvSetCharacterInventory();
			break;
		case DB_PROTOCOL::DBSERVER_RES_SET_ITEM_INVENTORY:
			manager->recvSetItemInventory();
			break;
		case DB_PROTOCOL::DBSERVER_RES_SET_COSTUME_INVENTORY:
			manager->recvSetCostumeInventory();
			break;
		case DB_PROTOCOL::DBSERVER_RES_SET_EMOTION_INVENTORY:
			manager->recvSetEmotionInventory();
			break;

		case DB_PROTOCOL::DBSERVER_RES_BASICINFO:
			manager->recvBasicInfo();
			break;
		case DB_PROTOCOL::DBSERVER_RES_MATCHINFO:
			manager->recvMatchInfo();
			break;
		case DB_PROTOCOL::DBSERVER_RES_CHARACTER_DACK:
			manager->recvCharacterDack();
			break;
		case DB_PROTOCOL::DBSERVER_RES_ITEM_DACK:
			manager->recvItemDack();
			break;
		case DB_PROTOCOL::DBSERVER_RES_EMOTION_DACK:
			manager->recvEmotionDack();
			break;
		case DB_PROTOCOL::DBSERVER_RES_CHARACTER_INVENTORY:
			manager->recvCharacterInventory();
			break;
		case DB_PROTOCOL::DBSERVER_RES_ITEM_INVENTORY:
			manager->recvItemInventory();
			break;
		case DB_PROTOCOL::DBSERVER_RES_COSTUME_INVENTORY:
			manager->recvCostumeInventory();
			break;
		case DB_PROTOCOL::DBSERVER_RES_EMOTION_INVENTORY:
			manager->recvEmotionInventory();
			break;

		case DB_PROTOCOL::DBSERVER_RES_CHECK_USEREXIST:
			manager->recvCheckUserExist();
			break;

		case DB_PROTOCOL::DBSERVER_RES_REGISTER:
			manager->recvRegister();
			break;
		case DB_PROTOCOL::DBSERVER_RES_WITHDRAW:
			manager->recvWithDraw();
			break;
		}
	}
	return 0;
}

/* Send */
void DBServerManager::sendSetBasicInfo(User& pUser)
{
	// [ string ID, string nickname, int LV, int EXP, int gold, int gem, int gemOfVictory, int costume ]
	UserData& d = pUser.getUserData();

	SendPacket packet(80);
	packet << DB_PROTOCOL::GAMESERVER_REQ_SET_BASICINFO;
	packet << d.id << d.nick << d.basic.lv << d.basic.exp << d.basic.gold << d.basic.gem << d.basic.gemOfVictory << d.basic.costume;

	client.sendMsg(packet);
};
void DBServerManager::sendSetMatchInfo(User& pUser)
{
	// [ string ID, int matchingCount, int victoryCount, int victoryStreak, int loseStreak, int score ]
	UserData& d = pUser.getUserData();

	SendPacket packet(80);
	packet << DB_PROTOCOL::GAMESERVER_REQ_SET_MATCHINFO;
	packet << d.id << d.basic.matchingCount << d.basic.victoryCount << d.basic.victoryStreak << d.basic.loseStreak << d.basic.score;

	client.sendMsg(packet);
};
void DBServerManager::sendSetCharacterDack(User& pUser)
{
	// [ string ID, int subCharacter1, int subCharacter2, int subCharacter3 ]
	UserData& d = pUser.getUserData();

	SendPacket packet(80);
	packet << DB_PROTOCOL::GAMESERVER_REQ_SET_CHARACTER_DACK;
	packet << d.id << d.character.leaderCostume << d.character.subCharacterSlot[0] << d.character.subCharacterSlot[1] << d.character.subCharacterSlot[2];

	client.sendMsg(packet);
};
void DBServerManager::sendSetItemDack(User& pUser)
{
	// [string ID, int itemSlot1, int itemSlot2, int itemSlot3]
	UserData& d = pUser.getUserData();

	SendPacket packet(80);
	packet << DB_PROTOCOL::GAMESERVER_REQ_SET_ITEM_DACK;
	packet << d.id << d.item.itemSlot[0] << d.item.itemSlot[1] << d.item.itemSlot[2];

	client.sendMsg(packet);
};
void DBServerManager::sendSetEmotionDack(User& pUser)
{
	// [ string ID, int emotionSlot1, int emotionSlot2, int emotionSlot3 ]
	UserData& d = pUser.getUserData();

	SendPacket packet(80);
	packet << DB_PROTOCOL::GAMESERVER_REQ_SET_EMOTION_DACK;
	packet << d.id << d.emotion.emotionSlot[0] << d.emotion.emotionSlot[1] << d.emotion.emotionSlot[2];

	client.sendMsg(packet);
};
void DBServerManager::sendSetCharacterInventory(User& pUser)
{
	// [ string ID, int recordCnt, vector<(int idx, int costumeIdx)> ]
	// 수정 => [ string ID, int recordCnt, costumeIdx X recordCnt ]
	UserData& d = pUser.getUserData();
	int recordCnt = d.character.configCostume.size();

	SendPacket packet(100);
	packet << DB_PROTOCOL::GAMESERVER_REQ_SET_CHARACTER_INVENTORY;
	packet << d.id << recordCnt;

	for (int i = 0; i < recordCnt; i++)
		packet << d.character.configCostume[i];

	client.sendMsg(packet);

};
void DBServerManager::sendSetCostumeInventory(User& pUser)
{
	UserData& d = pUser.getUserData();

	SendPacket packet(80);
	packet << DB_PROTOCOL::GAMESERVER_REQ_SET_COSTUME_INVENTORY;
	packet << d.id << (int)d.character.costumes.size();
	for (auto data : d.character.costumes)
	{
		packet << data;
	}

	client.sendMsg(packet);
};
void DBServerManager::sendSetItemInventory(User& pUser)
{
	UserData& d = pUser.getUserData();

	SendPacket packet(100);
	packet << DB_PROTOCOL::GAMESERVER_REQ_SET_ITEM_INVENTORY;
	packet << d.id << (int)d.item.items.size();
	for (int data : d.item.items)
	{
		packet << data;
	}

	client.sendMsg(packet);
};
void DBServerManager::sendSetEmotionInventory(User& pUser)
{
	UserData& d = pUser.getUserData();

	SendPacket packet(100);
	packet << DB_PROTOCOL::GAMESERVER_REQ_SET_EMOTION_INVENTORY;
	packet << d.id << (int)d.emotion.emotions.size();
	for (int data : d.emotion.emotions)
	{
		packet << data;
	}

	client.sendMsg(packet);
};

void DBServerManager::sendBasicInfo(User& pUser)
{
	// [ string ID ]
	SendPacket packet(30);

	packet << DB_PROTOCOL::GAMESERVER_REQ_BASICINFO;
	packet << pUser.getUserData().id;

	client.sendMsg(packet);
};
void DBServerManager::sendMatchInfo(User& pUser)
{
	// [ string ID ]
	SendPacket packet(30);

	packet << DB_PROTOCOL::GAMESERVER_REQ_MATCHINFO;
	packet << pUser.getUserData().id;

	client.sendMsg(packet);
};
void DBServerManager::sendCharacterDack(User& pUser)
{
	// [ string ID ]
	SendPacket packet(30);

	packet << DB_PROTOCOL::GAMESERVER_REQ_CHARACTER_DACK;
	packet << pUser.getUserData().id;

	client.sendMsg(packet);
};
void DBServerManager::sendItemDack(User& pUser)
{
	// [ string ID ]
	SendPacket packet(30);

	packet << DB_PROTOCOL::GAMESERVER_REQ_ITEM_DACK;
	packet << pUser.getUserData().id;

	client.sendMsg(packet);
};
void DBServerManager::sendEmotionDack(User& pUser)
{
	// [ string ID ]
	SendPacket packet(30);

	packet << DB_PROTOCOL::GAMESERVER_REQ_EMOTION_DACK;
	packet << pUser.getUserData().id;

	client.sendMsg(packet);
};
void DBServerManager::sendCostumeInventory(User& pUser)
{
	// [ string ID ]
	SendPacket packet(30);

	packet << DB_PROTOCOL::GAMESERVER_REQ_COSTUME_INVENTORY;
	packet << pUser.getUserData().id;

	client.sendMsg(packet);
};
void DBServerManager::sendItemInventory(User& pUser)
{
	SendPacket packet(30);

	packet << DB_PROTOCOL::GAMESERVER_REQ_ITEM_INVENTORY;
	packet << pUser.getUserData().id;

	client.sendMsg(packet);
};
void DBServerManager::sendEmotionInventory(User& pUser)
{
	SendPacket packet(30);

	packet << DB_PROTOCOL::GAMESERVER_REQ_EMOTION_INVENTORY;
	packet << pUser.getUserData().id;

	client.sendMsg(packet);
};
void DBServerManager::sendCharacterInventory(User& pUser)
{
	SendPacket packet(30);

	packet << DB_PROTOCOL::GAMESERVER_REQ_CHARACTER_INVENTORY;
	packet << pUser.getUserData().id;

	client.sendMsg(packet);
};
void DBServerManager::sendReq(User& pUser, UserData::USER_DATA userData)
{
	SendPackets packets;

	SendPacket packet(30), packet2(30), packet3(30), packet4(30), packet5(30), packet6(30), packet7(30), packet8(30), packet9(30);
	if ((userData & UserData::UD_BASICINFO) != 0)
	{
		packet << DB_PROTOCOL::GAMESERVER_REQ_BASICINFO << pUser.getUserData().id;
		packets += packet;
	}
	if ((userData & UserData::UD_MATCHINFO) != 0)
	{
		packet2 << DB_PROTOCOL::GAMESERVER_REQ_MATCHINFO << pUser.getUserData().id;
		packets += packet2;
	}
	if ((userData & UserData::UD_CHARACTER_DACK) != 0)
	{
		packet3 << DB_PROTOCOL::GAMESERVER_REQ_CHARACTER_DACK << pUser.getUserData().id;
		packets += packet3;
	}
	if ((userData & UserData::UD_ITEM_DACK) != 0)
	{
		packet4 << DB_PROTOCOL::GAMESERVER_REQ_ITEM_DACK << pUser.getUserData().id;
		packets += packet4;
	}
	if ((userData & UserData::UD_EMOTION_DACK) != 0)
	{
		packet5 << DB_PROTOCOL::GAMESERVER_REQ_EMOTION_DACK << pUser.getUserData().id;
		packets += packet5;
	}
	if ((userData & UserData::UD_CHARACTER_INVENTORY) != 0)
	{
		packet6 << DB_PROTOCOL::GAMESERVER_REQ_CHARACTER_INVENTORY << pUser.getUserData().id;
		packets += packet6;
	}
	if ((userData & UserData::UD_ITEM_INVENTORY) != 0)
	{
		packet7 << DB_PROTOCOL::GAMESERVER_REQ_ITEM_INVENTORY << pUser.getUserData().id;
		packets += packet7;
	}
	if ((userData & UserData::UD_COSTUME_INVENTORY) != 0)
	{
		packet8 << DB_PROTOCOL::GAMESERVER_REQ_COSTUME_INVENTORY << pUser.getUserData().id;
		packets += packet8;
	}
	if ((userData & UserData::UD_EMOTION_INVENTORY) != 0)
	{
		packet9 << DB_PROTOCOL::GAMESERVER_REQ_EMOTION_INVENTORY << pUser.getUserData().id;
		packets += packet9;
	}
	client.sendMsg(packets);
}

void DBServerManager::sendCheckUserExist(User& pUser)
{
	// [ string ID ]
	SendPacket packet(30);

	packet << DB_PROTOCOL::GAMESERVER_REQ_CHECK_USEREXIST;
	packet << pUser.getUserData().id;

	client.sendMsg(packet);
};

void DBServerManager::sendRegister(User& pUser)
{
	// [ string ID, string nickname ]
	SendPacket packet(100);

	packet << DB_PROTOCOL::GAMESERVER_REQ_REGISTER;
	packet << pUser.getUserData().id << pUser.getUserData().nick;

	client.sendMsg(packet);
};
void DBServerManager::sendWithDraw(User& pUser)
{
	// [ string ID ]
	SendPacket packet(30);

	packet << DB_PROTOCOL::GAMESERVER_REQ_WITHDRAW;
	packet << pUser.getUserData().id;

	client.sendMsg(packet);
};

/* Recv */
void DBServerManager::recvSetBasicInfo()
{
	// [string ID, bool isSuccess]
	std::string id;
	client >> id;

	bool isSuccess = false;
	client >> isSuccess;

	//printf("CHANGE %s\'s BASICINFO: ", id.c_str());
	//printf("%s\n", (isSuccess) ? "TRUE" : "FALSE");
}
void DBServerManager::recvSetMatchInfo()
{
	// [string ID, bool isSuccess]
	std::string id;
	client >> id;

	bool isSuccess = false;
	client >> isSuccess;

	//printf("CHANGE %s\'s MATCHINFO: ", id.c_str());
	//printf("%s\n", (isSuccess) ? "TRUE" : "FALSE");
}
void DBServerManager::recvSetCharacterDack()
{
	// [string ID, bool isSuccess]
	std::string id;
	client >> id;

	bool isSuccess = false;
	client >> isSuccess;

	//printf("CHANGE %s\'s CHARACTER_DACK: ", id.c_str());
	//printf("%s\n", (isSuccess) ? "TRUE" : "FALSE");
}
void DBServerManager::recvSetItemDack()
{
	// [string ID, bool isSuccess]
	std::string id;
	client >> id;

	bool isSuccess = false;
	client >> isSuccess;

	//printf("CHANGE %s\'s ITEM_DACK: ", id.c_str());
	//printf("%s\n", (isSuccess) ? "TRUE" : "FALSE");
};
void DBServerManager::recvSetEmotionDack()
{
	// [string ID, bool isSuccess]
	std::string id;
	client >> id;

	bool isSuccess = false;
	client >> isSuccess;

	//printf("CHANGE %s\'s EMOTION_DACK: ", id.c_str());
	//printf("%s\n", (isSuccess) ? "TRUE" : "FALSE");
};
void DBServerManager::recvSetCharacterInventory()
{
	// [ string ID, bool isSuccess ]
	std::string id;
	client >> id;

	bool isSuccess = false;
	client >> isSuccess;

	//printf("CHANGE %s\'s CHARACTER_INVENTORY: ", id.c_str());
	//printf("%s\n", (isSuccess) ? "TRUE" : "FALSE");
}
void DBServerManager::recvSetCostumeInventory()
{
	// [ string ID, bool isSuccess ]
	std::string id;
	client >> id;

	bool isSuccess = false;
	client >> isSuccess;

	//printf("CHANGE %s\'s COSTUME_INVENTORY: ", id.c_str());
	//printf("%s\n", (isSuccess) ? "TRUE" : "FALSE");
};
void DBServerManager::recvSetItemInventory()
{
	// [ string ID, bool isSuccess ]
	std::string id;
	client >> id;

	bool isSuccess = false;
	client >> isSuccess;

	//printf("CHANGE %s\'s ITEM_INVENTORY: ", id.c_str());
	//printf("%s\n", (isSuccess) ? "TRUE" : "FALSE");
};
void DBServerManager::recvSetEmotionInventory()
{
	// [ string ID, bool isSuccess ]
	std::string id;
	client >> id;

	bool isSuccess = false;
	client >> isSuccess;

	//printf("CHANGE %s\'s EMOTION_INVENTORY: ", id.c_str());
	//printf("%s\n", (isSuccess) ? "TRUE" : "FALSE");
};

void DBServerManager::recvBasicInfo()
{
	// [ string ID, string nickname, int LV, int EXP, int gold, int gem, int gemOfVictory, int costume ]
	std::string id;
	client >> id;

	DataBaseManager::GetInstance()->EnterCS();
	User* user = DataBaseManager::GetInstance()->getUser(id);
	if (user == nullptr) {
		DataBaseManager::GetInstance()->LeaveCS(); return;
	}
	UserData& data = user->getUserData();
	client >> data.nick >> data.basic.lv >> data.basic.exp >> data.basic.gold >> data.basic.gem >> data.basic.gemOfVictory >> data.basic.costume;

	user->recvUserData(UserData::UD_BASICINFO);

	DataBaseManager::GetInstance()->LeaveCS();
};
void DBServerManager::recvMatchInfo()
{
	// [ string ID, int matchingCount, int victoryCount, int victoryStreak, int loseStreak, int score ]
	std::string id;
	client >> id;

	DataBaseManager::GetInstance()->EnterCS();
	User* user = DataBaseManager::GetInstance()->getUser(id);
	if (user == nullptr) {
		DataBaseManager::GetInstance()->LeaveCS(); return;
	}
	UserData& data = user->getUserData();
	client >> data.basic.matchingCount >> data.basic.victoryCount >> data.basic.victoryStreak >> data.basic.loseStreak >> data.basic.score;

	user->recvUserData(UserData::UD_MATCHINFO);

	DataBaseManager::GetInstance()->LeaveCS();
};
void DBServerManager::recvCharacterDack()
{
	// [ string ID, int subCharacter1, int subCharacter2, int subCharacter3 ]
	std::string id;
	client >> id;

	DataBaseManager::GetInstance()->EnterCS();
	User* user = DataBaseManager::GetInstance()->getUser(id);
	if (user == nullptr) {
		DataBaseManager::GetInstance()->LeaveCS(); return;
	}
	UserData& data = user->getUserData();
	client >> data.character.leaderCostume >> data.character.subCharacterSlot[0] >> data.character.subCharacterSlot[1] >> data.character.subCharacterSlot[2];

	user->recvUserData(UserData::UD_CHARACTER_DACK);

	DataBaseManager::GetInstance()->LeaveCS();
};
void DBServerManager::recvItemDack()
{
	// [ string ID, int itemSlot1, int itemSlot2, int itemSlot3 ]
	std::string id;
	client >> id;

	DataBaseManager::GetInstance()->EnterCS();
	User* user = DataBaseManager::GetInstance()->getUser(id);
	if (user == nullptr) {
		DataBaseManager::GetInstance()->LeaveCS(); return;
	}
	UserData& data = user->getUserData();
	client >> data.item.itemSlot[0] >> data.item.itemSlot[1] >> data.item.itemSlot[2];

	user->recvUserData(UserData::UD_ITEM_DACK);

	DataBaseManager::GetInstance()->LeaveCS();
};
void DBServerManager::recvEmotionDack()
{
	// [ string ID, int emotionSlot1, int emotionSlot2, int emotionSlot3 ]
	std::string id;
	client >> id;

	DataBaseManager::GetInstance()->EnterCS();
	User* user = DataBaseManager::GetInstance()->getUser(id);
	if (user == nullptr) {
		DataBaseManager::GetInstance()->LeaveCS(); return;
	}
	UserData& data = user->getUserData();
	client >> data.emotion.emotionSlot[0] >> data.emotion.emotionSlot[1] >> data.emotion.emotionSlot[2];

	user->recvUserData(UserData::UD_EMOTION_DACK);

	DataBaseManager::GetInstance()->LeaveCS();
};
void DBServerManager::recvCharacterInventory()
{
	std::string id;
	client >> id;

	DataBaseManager::GetInstance()->EnterCS();
	User* user = DataBaseManager::GetInstance()->getUser(id);
	if (user == nullptr) {
		DataBaseManager::GetInstance()->LeaveCS(); return;
	}
	UserData& data = user->getUserData();

	int cnt = 0;
	client >> cnt;
	data.character.configCostume.resize(cnt);

	for (int i = 0; i < cnt; i++)
	{
		client >> data.character.configCostume[i];
	}

	user->recvUserData(UserData::UD_CHARACTER_INVENTORY);

	DataBaseManager::GetInstance()->LeaveCS();
};
void DBServerManager::recvCostumeInventory()
{
	std::string id;
	client >> id;

	DataBaseManager::GetInstance()->EnterCS();
	User* user = DataBaseManager::GetInstance()->getUser(id);
	if (user == nullptr) {
		DataBaseManager::GetInstance()->LeaveCS(); return;
	}
	UserData& data = user->getUserData();

	int cnt = 0;
	client >> cnt;
	data.character.costumes.resize(cnt);

	for (int i = 0; i < cnt; i++)
	{
		client >> data.character.costumes[i];
	}

	user->recvUserData(UserData::UD_COSTUME_INVENTORY);

	DataBaseManager::GetInstance()->LeaveCS();
};
void DBServerManager::recvItemInventory()
{
	std::string id;
	client >> id;

	DataBaseManager::GetInstance()->EnterCS();
	User* user = DataBaseManager::GetInstance()->getUser(id);
	if (user == nullptr) {
		DataBaseManager::GetInstance()->LeaveCS(); return;
	}
	UserData& data = user->getUserData();

	int cnt = 0;
	client >> cnt;
	data.item.items.resize(cnt);

	for (int i = 0; i < cnt; i++)
	{
		client >> data.item.items[i];
	}

	user->recvUserData(UserData::UD_ITEM_INVENTORY);

	DataBaseManager::GetInstance()->LeaveCS();
};
void DBServerManager::recvEmotionInventory()
{
	std::string id;
	client >> id;

	DataBaseManager::GetInstance()->EnterCS();
	User* user = DataBaseManager::GetInstance()->getUser(id);
	if (user == nullptr) {
		DataBaseManager::GetInstance()->LeaveCS(); return;
	}
	UserData& data = user->getUserData();

	int cnt = 0;
	client >> cnt;
	data.emotion.emotions.resize(cnt);

	for (int i = 0; i < cnt; i++)
	{
		client >> data.emotion.emotions[i];
	}

	user->recvUserData(UserData::UD_EMOTION_INVENTORY);

	DataBaseManager::GetInstance()->LeaveCS();
};

void DBServerManager::recvCheckUserExist()
{
	// [ string ID, bool isExist ]
	std::string id;
	client >> id;

	bool isExist = false;
	client >> isExist;

	DataBaseManager::GetInstance()->EnterCS();
	User* user = DataBaseManager::GetInstance()->getUser(id);
	if (user == nullptr) {
		DataBaseManager::GetInstance()->LeaveCS(); return;
	}

	// 로그인
	user->EnterCS();
	LoginLicence* licence = user->getLoginLicence();
	bool isCheck = user->isStateOf(USER_STATE::CHECK);

	if (isCheck) // 유저 상태 : 체크 스테이트
	{
		if (licence->isLoginReq) {// 로그인 요청
			bool isGoogle = (user->getUserData().id.substr(0, 1) == "1");
			bool sameID = (licence->id == id && isExist);

			if (isGoogle) {// 구글
				SendPacket sp;
				sp << SERVER_GOOGLE_LOGIN << sameID;
				user->sendMsg(sp);

				if (sameID) {
					licence->isLoginReq = true;
					user->setState(USER_STATE::LOBBY);
					user->setIsLogin(true);
					DBServerManager::GetInstance()->sendReq(*user, UserData::UD_ALL);
				}
			}
			else {// 게스트

			}
		}
		else {// 회원가입 체크
			SendPacket sp;
			sp << SERVER_GOOGLE_JOIN_CHECK << isExist;
			user->sendMsg(sp);
		}
	}

	user->LeaveCS();

	DataBaseManager::GetInstance()->LeaveCS();
};

void DBServerManager::recvRegister()
{
	// [string ID, bool isSuccess]
	std::string id;
	client >> id;

	bool success = false;
	client >> success;

	DataBaseManager::GetInstance()->EnterCS();
	User* user = DataBaseManager::GetInstance()->getUser(id);
	if (user == nullptr) {
		DataBaseManager::GetInstance()->LeaveCS(); return;
	}

	SendPacket sp;
	sp << SERVER_GOOGLE_JOIN << success;
	user->sendMsg(sp);

	DataBaseManager::GetInstance()->LeaveCS();
};
void DBServerManager::recvWithDraw()
{
	// [string ID, bool isSuccess]
	std::string id;
	client >> id;

	bool isExist = false;
	client >> isExist;

	//printf("WITHDRAW: ", id.c_str());
	//printf("%s\n", (isExist) ? "TRUE" : "FALSE");
};