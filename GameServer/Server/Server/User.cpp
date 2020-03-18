#include "User.h"
#include "DataBase.h"
#include "MatchingManager.h"
#include "MatchingState.h"
#include "LicenceState.h"
#include "DebugClientState.h"
#include "CheckState.h"
#include "LobbyState.h"
#include "DebugClientManager.h"
#include "DBServerManager.h"
#include "Encoding.h"

#include <time.h>
#include <stdio.h>
#include <iostream>

using namespace std;

User::User()
{
	static int _serial = 0;
	currentState = nullptr;
	serial = ++_serial;
	_avg_ping_sub = _avg_ping_count = ping = 0;
	_avg_ping = pong = -1;
	isLogin = false;
	InitializeCriticalSection(&cs);
}
User::~User()
{
	if (currentState != nullptr)
	{
		delete currentState;
		currentState = nullptr;
	}
	DeleteCriticalSection(&cs);
}

void User::recvProcess()
{
	if (currentState != nullptr)
		currentState->recv(this);
}
void User::acceptProcess()
{
	char ip[100]; getIP(ip);
	{// 디버그용
		time_t r;
		struct tm * ti;
		time(&r);
		ti = localtime(&r);

		printf("#%d.%d.%d %d:%d:%d# Client %d [%s] connect\n", ti->tm_year + 1900, ti->tm_mon + 1, ti->tm_mday, ti->tm_hour, ti->tm_min, ti->tm_sec, getSerial(), ip);
		//delete ti;
	}
	setState(USER_STATE::LICENCE);
	DataBaseManager::GetInstance()->addUser(this);

	srand(time(0));	rand();

	/*data.id = "날씬한정현우2";
	//DBServerManager::GetInstance()->sendCheckUserExist(*this);
	//DBServerManager::GetInstance()->sendRegister(*this);
	DBServerManager::GetInstance()->sendReq(*this, UserData::UD_ALL);*/
}
void User::disconnectProcess()
{
	if (currentState != nullptr)
		currentState->disconnect(this);

	DataBaseManager::GetInstance()->removeUser(this);
	DebugClientManager::GetInstance()->sendDisconnectUser(this);

	char ip[100]; getIP(ip);
	{// 디버그용
		time_t r;
		struct tm * ti;
		time(&r);
		ti = localtime(&r);

		printf("#%d.%d.%d %d:%d:%d# Client %d [%s] disconnect\n", ti->tm_year + 1900, ti->tm_mon + 1, ti->tm_mday, ti->tm_hour, ti->tm_min, ti->tm_sec, getSerial(), ip);
		//delete ti;
	}
	//printf("Client %d [%s] disconnect\n",getSerial(), ip);
	delete this;
}
void User::errorProcess(IOCP_ERROR error) 
{
	switch (error)
	{
	case IOCPERROR_NONE:
		return;
	case TOTALSIZE_MINUS:
		cout << "IOCP Error : TOTALSIZE_MINUS" << endl;
		break;
	case TOTALSIZE_TOOBIG:
		cout << "IOCP Error : TOTALSIZE_TOOBIG" << endl;
		break;
	}
	disconnectProcess();
}
bool User::setMatching(Matching* matching)
{// setMatching은 하나의 쓰레드에서만 접근하는 것이 확실. 고로 크리티컬 섹션X
	MatchingState* state = dynamic_cast<MatchingState*>(currentState);
	if (state == nullptr)
	{
		return false;
	}
	state->setMatching(matching);
	return true;
}
Matching* User::getMatching()
{
	EnterCS();
	if (currentState == nullptr) {
		LeaveCS();
		return nullptr;
	}
	if (currentState->getStateType() == USER_STATE::MATCHING)
	{
		Matching* m = ((MatchingState*)currentState)->getMatching();
		LeaveCS();
		return m;
	}
	LeaveCS();
	return nullptr;
}
LoginLicence* User::getLoginLicence()
{
	EnterCS();
	if (currentState == nullptr) {
		LeaveCS();
		return nullptr;
	}
	if (currentState->getStateType() == USER_STATE::CHECK)
	{
		LoginLicence* m = &((CheckState*)currentState)->licence;
		LeaveCS();
		return m;
	}
	LeaveCS();
	return nullptr;
}
UserData& User::getUserData()
{
	return data;
}
bool User::isDebugClient()
{
	EnterCS();
	if (currentState->getStateType() == USER_STATE::DEBUG)
	{
		LeaveCS();
		return true;
	}
	LeaveCS();
	return false;
}
bool User::isGameClient()
{
	EnterCS();
	switch (currentState->getStateType())
	{
	case LICENCE:
	case DEBUG:
		LeaveCS();
		return false;
	case CHECK:
	case LOBBY:
	case MATCHING:
		LeaveCS();
		return true;
	}
	LeaveCS();
	return false;
}
bool User::isStateOf(USER_STATE state)
{
	if (currentState == nullptr) return false;
	return (currentState->getStateType() == state);
}
int User::getSerial()
{
	return serial;
}
State* User::getStateObject()
{
	return currentState;
}
USER_STATE User::getStateType()
{
	return currentState->getStateType();
}
void User::setState(USER_STATE state)
{
	State* deleteState = nullptr;
	State* replace = nullptr;

	EnterCS();
	switch (state) {
	case USER_STATE::LICENCE:
		replace = new LicenceState();
		break;
	case USER_STATE::CHECK:
		replace = new CheckState();
		break;
	case USER_STATE::LOBBY:
		replace = new LobbyState();
		break;
	case USER_STATE::MATCHING:
		replace = new MatchingState();
		break;
	case USER_STATE::DEBUG:
		replace = new DebugClientState();
		break;
	}
	
	deleteState = currentState;
	currentState = replace;
	if (deleteState != nullptr)
	{
		delete deleteState;
	}
	LeaveCS();

	DebugClientManager::GetInstance()->sendConnectUser(this);
}
void User::EnterCS()
{
	EnterCriticalSection(&cs);
}
void User::LeaveCS()
{
	LeaveCriticalSection(&cs);
}
short User::getPing()
{
	return pong;
}
short User::getAvgPing()
{
	return _avg_ping;
}
bool User::getIsLogin()
{
	return isLogin;
}
void User::setPong()
{
	pong = clock() - ping;
	_avg_ping_sub += pong;
	_avg_ping_count++;
	if (_avg_ping_count >= 10) {
		_avg_ping = _avg_ping_sub / _avg_ping_count;
		_avg_ping_sub = 0;
		_avg_ping_count = 0;
	}
}
void User::setPing()
{
	ping = clock();
}
void User::setIsLogin(bool pIsLogin)
{
	isLogin = pIsLogin;
}

void User::recvUserData(UserData::USER_DATA _data)
{
	data.recvCheck |= _data;
	if (data.recvCheck == UserData::UD_ALL) {
		data.recvCheck = UserData::UD_NONE;
		SendPacket sp(300);
		UserBasicData& basic = data.basic;
		UserItemData& item = data.item;
		UserCharacterData& crt = data.character;
		UserEmotionData& emotion = data.emotion;
		
		sp << SERVER_USERDATA;

		wchar_t *wid, *wnick;
		Encoding::AsciiToUnicode(data.id.data(), wid);
		Encoding::AsciiToUnicode(data.nick.data(), wnick);
		sp << wid << wnick;
		delete wid;
		delete wnick;

		sp << basic.lv << basic.exp << basic.gold << basic.gem;
		sp << basic.matchingCount << basic.victoryCount << basic.victoryStreak;
		sp << basic.gemOfVictory << basic.costume;
		sp << basic.loseStreak << basic.score;

		sp << item.itemSlot[0] << item.itemSlot[1] << item.itemSlot[2] << (int)item.items.size();
		for (int d : item.items)			sp << d;

		sp << crt.subCharacterSlot[0] << crt.subCharacterSlot[1] << crt.subCharacterSlot[2] << crt.leaderCostume;
		sp << (int)crt.configCostume.size();
		for (int d : crt.configCostume)		sp << d;
		sp << (int)crt.costumes.size();
		for (int d : crt.costumes)			sp << d;

		//sp << emotion.emotionSlot[0] << emotion.emotionSlot[1] << emotion.emotionSlot[2] << (int)emotion.emotions.size();
		//for (int d : emotion.emotions)		sp << d;

		sendMsg(sp);
	}
}