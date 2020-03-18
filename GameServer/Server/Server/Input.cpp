#include "User.h"
#include "Input.h"
#include <iostream>
#include <string>
#include "DataBase.h"
#include "StaticGameData.h"
#include "Matching.h"
#include "MainManager.h"
#include "IOCPServer.h"
#include "TimerManager.h"

Input* Input::instance = nullptr;

Input::Input()
{
	flag = new bool;
	*flag = true;
	CreateThread(0, 0, process, flag, 0, 0);
}
Input::~Input()
{
	*flag = false;
}
Input* Input::CreateInstance()
{
	if (instance == nullptr)
	{
		instance = new Input();
	}
	return instance;
}
Input* Input::GetInstance()
{
	return instance;
}
void Input::DestroyInstance()
{
	if (instance != nullptr)
	{
		delete instance;
		instance = nullptr;
	}
}
DWORD WINAPI Input::process(void* v)
{
	using namespace std;

	bool* flag = (bool*)v;

	string input;
	while (flag) 
	{
		cin >> input;

		// csv 다시 로딩 명령어
		// 실제 서비스할 때 실행은 금물! 디버그에서만 사용!!
		if (input == "/csv")
		{
			cout << "command: reload csv" << endl;
			DataBaseManager::GetInstance()->getGameData().load();
		}
		else if (input == "/clear") {
			system("cls");
		}
		else if (input == "/client") {
			cout << "command: show client info" << endl;
			int count = DataBaseManager::GetInstance()->getUserCount();
			char ip[50];
			for (int i = 0; i < count; i++) {
				User* user = DataBaseManager::GetInstance()->getUser(i);
				user->getIP(ip);
				cout << "[" << ip << "] client " << user->getSerial();
				if (user->isGameClient()) {
					cout << "\tping (" << user->getPing() << ", avg " << user->getAvgPing() << ")";
				}
				if (user->isDebugClient()) {
					cout << "\tDebug Client";
				}
				if (user->getStateType() == USER_STATE::MATCHING && user->getMatching() != nullptr) {
					cout << "\tmatching " << user->getMatching()->getSerial();
				}
				cout << endl;
			}
			cout << "Client count : " << DataBaseManager::GetInstance()->getUserCount() << endl;
			cout << "Matching count : " << DataBaseManager::GetInstance()->getMatchingCount() << endl;
		}
		else if (input == "/thread") {
			cout << "command: show thread info" << endl;
			int server_count = MainManager::GetInstance()->getServer()->getThreadCount();
			int timer_count = TimerManager::GetInstance()->getThreadCount();
			cout << "Main thread : 1" << endl;
			cout << "Input thread : 1" << endl;
			cout << "DBServer thread : 1" << endl;
			cout << "IOCPServer thread : " << server_count << endl;
			cout << "TimerManager thread : " << timer_count << endl;
			cout << "=> Total thread count : " << (3 + server_count + timer_count) << endl;
		}
		else if (input == "/?") {
			cout << "* command *" << endl;
			cout << "/csv : csv 다시 로드" << endl;
			cout << "/clear : 화면 초기화" << endl;
			cout << "/client : 접속 클라이언트 정보" << endl;
			cout << "/thread : 실행중인 쓰레드 정보" << endl;
		}
		else {
			cout << "invalid command" << endl;
		}
	}
	delete flag;
	return 0;
}