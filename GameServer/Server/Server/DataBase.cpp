#include "User.h"
#include "DataBase.h"
#include "GlobalProtocol.h"
#include <time.h>

DataBaseManager* DataBaseManager::Instance = nullptr;


DataBaseManager::DataBaseManager() 
{
	InitializeCriticalSection(&cs); 

	TimerManager::GetInstance()->addTimer(this, 1000);
}
DataBaseManager::~DataBaseManager() 
{
	TimerManager::GetInstance()->removeTimer(this, 1000);
	DeleteCriticalSection(&cs); 
}


DataBaseManager* DataBaseManager::CreateInstance()
{
	if (Instance == nullptr)
	{
		Instance = new DataBaseManager();
	}
	return Instance;
}
DataBaseManager* DataBaseManager::GetInstance()
{
	return Instance;
}
void DataBaseManager::DestroyInstance()
{
	if (Instance != nullptr)
	{
		delete Instance;
	}
}
void DataBaseManager::EnterCS()
{
	EnterCriticalSection(&cs);
}
void DataBaseManager::LeaveCS()
{
	LeaveCriticalSection(&cs);
}

void DataBaseManager::addUser(User* user)
{
	EnterCriticalSection(&cs);
	userList.push_back(user);
	LeaveCriticalSection(&cs);
}
void DataBaseManager::removeUser(User* user)
{
	EnterCriticalSection(&cs);
	int count = userList.size();
	for (int i = 0; i < count; i++)
	{
		if (userList[i] == user)
		{
			userList.erase(userList.begin() + i);
			break;
		}
	}
	LeaveCriticalSection(&cs);
}
void DataBaseManager::addMatching(Matching* matching)
{
	EnterCriticalSection(&cs);
	matchingList.push_back(matching);
	LeaveCriticalSection(&cs);
}
#include "Matching.h"
bool DataBaseManager::removeMatching(Matching* matching)
{
	EnterCriticalSection(&cs);
	int count = matchingList.size();
	for (int i = 0; i < count; i++)
	{
		if (matchingList[i] == matching)
		{
			matchingList.erase(matchingList.begin() + i);
			LeaveCriticalSection(&cs);
			return true;
		}
	}
	LeaveCriticalSection(&cs);
	return false;
}

void DataBaseManager::addDebugUser(User* user)
{
	EnterCriticalSection(&cs);
	debugUserList.push_back(user);
	LeaveCriticalSection(&cs);
}
void DataBaseManager::removeDebugUser(int index)
{
	EnterCriticalSection(&cs);
	debugUserList.erase(debugUserList.begin() + index);
	LeaveCriticalSection(&cs);
}
void DataBaseManager::removeDebugUser(User* user)
{
	EnterCriticalSection(&cs);
	int count = debugUserList.size();
	for (int i = 0; i < count; i++) {
		if (debugUserList[i] == user) {
			removeDebugUser(i);
			break;
		}
	}
	LeaveCriticalSection(&cs);
}
int DataBaseManager::getDebugUserCount()
{
	return debugUserList.size();
}
int DataBaseManager::getUserCount()
{
	return userList.size();
}
User* DataBaseManager::getUser(int index)
{
	return userList[index];
}
User* DataBaseManager::getUser(std::string pID)
{
	EnterCriticalSection(&cs);
	for (auto user : userList)
	{
		if (user->getUserData().id == pID)
		{
			LeaveCriticalSection(&cs);
			return user;
		}
	}

	LeaveCriticalSection(&cs);
	return nullptr;
}
User* DataBaseManager::getDebugUser(int index)
{
	return debugUserList[index];
}
int DataBaseManager::getMatchingCount()
{
	return matchingList.size();
}
Matching* DataBaseManager::getMatching(int index)
{
	return matchingList[index];
}
StaticGameData& DataBaseManager::getGameData()
{
	return gameData;
}
bool DataBaseManager::contain(User* user)
{
	bool isContain = false;
	EnterCriticalSection(&cs);

	for (auto _user : userList)
		if (_user == user)
		{
			isContain = true; break;
		}

	LeaveCriticalSection(&cs);
	return isContain;
}
bool DataBaseManager::contain(Matching* matching)
{
	bool isContain = false;
	EnterCriticalSection(&cs);

	for (auto m : matchingList)
		if (m == matching)
		{
			isContain = true; break;
		}

	LeaveCriticalSection(&cs);
	return isContain;
}
bool DataBaseManager::isLoggedIn(std::string id)
{
	bool loggedIn = false;
	EnterCriticalSection(&cs);

	for (auto user : userList)
		if (user->getIsLogin() && user->getUserData().id == id)
		{
			loggedIn = true; break;
		}

	LeaveCriticalSection(&cs);
	return loggedIn;
}
void DataBaseManager::process()
{
	SendPacket sp;
	sp << GAME_PROTOCOL::SERVER_PING;

	EnterCriticalSection(&cs);
	for (auto user : userList)
	{
		if (user->isGameClient() == false)
			continue;

		user->setPing();
		user->sendMsg(sp);
	}
	LeaveCriticalSection(&cs);
}