#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <vector>
#include <Windows.h>
#include "Global.h"
#include "StaticGameData.h"
#include "TimerManager.h"
#include <string>

class User;
class Matching;

class DataBaseManager : public TimerProcess
{
private:
	static DataBaseManager* Instance;
	DataBaseManager();
	~DataBaseManager();

	CRITICAL_SECTION cs;

	std::vector<User*> userList;
	std::vector<User*> debugUserList;
	std::vector<Matching*> matchingList;
	StaticGameData gameData;

public:
	static DataBaseManager* CreateInstance();
	static DataBaseManager* GetInstance();
	static void DestroyInstance();

	void EnterCS();// �ֻ��� ũ��Ƽ�� ����. �ٸ� CS�� �ڽ��� �Ǹ� �ȵ�
	void LeaveCS();

	void addUser(User* user);
	void addMatching(Matching* matching);
	void addDebugUser(User* user);
	void removeUser(User* user);
	bool removeMatching(Matching* matching);
	void removeDebugUser(int index);
	void removeDebugUser(User* user);

	int getUserCount();
	User* getUser(int index);
	User* getUser(std::string pID);
	int getDebugUserCount();
	User* getDebugUser(int index);
	int getMatchingCount();
	Matching* getMatching(int index);

	bool contain(User* user);	// userList�� ������ �ִ���
	bool contain(Matching* matching);	// userList�� ������ �ִ���
	bool isLoggedIn(std::string id);	// �α��� �� ����?

	StaticGameData& getGameData();

	// pingüũ�� timer process
	void process() override;
};

#define db_matching	DataBaseManager::GetInstance()->getGameData().matching
#define db_gimmick	DataBaseManager::GetInstance()->getGameData().gimmick
#define db_leader	DataBaseManager::GetInstance()->getGameData().leader
#define db_sub		DataBaseManager::GetInstance()->getGameData().sub
#define db_skill	DataBaseManager::GetInstance()->getGameData().skill
#define db_item		DataBaseManager::GetInstance()->getGameData().item
#define db_costume	DataBaseManager::GetInstance()->getGameData().costume

#endif