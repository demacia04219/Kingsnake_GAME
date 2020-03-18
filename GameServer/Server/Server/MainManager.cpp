#include "MainManager.h"
#include "IOCPServer.h"
#include "MatchingManager.h"
#include "User.h"
#include "Global.h"
#include "Player.h"
#include "TimerManager.h"
#include "DataBase.h"
#include "LicenceManager.h"
#include "DebugClientManager.h"
#include "DBServerManager.h"
#include "CheckManager.h"
#include "LobbyManager.h"
#include "Input.h"
#include <time.h>

MainManager* MainManager::Instance = nullptr;

MainManager::MainManager()
{
	server = nullptr;
}
MainManager::~MainManager()
{
}

MainManager* MainManager::CreateInstance()
{
	if (Instance == nullptr)
	{
		Instance = new MainManager();

		TimerManager::CreateInstance();
		DBServerManager::CreateInstance();
		DataBaseManager::CreateInstance();
		MatchingManager::CreateInstance();
		LicenceManager::CreateInstance();
		DebugClientManager::CreateInstance();
		Input::CreateInstance();
		CheckManager::CreateInstance();
		LobbyManager::CreateInstance();
	}
	return Instance;
}
MainManager* MainManager::GetInstance()
{
	return Instance;
}
void MainManager::DestroyInstance()
{
	if (Instance != nullptr)
	{
		LobbyManager::DestroyInstance();
		CheckManager::DestroyInstance();
		Input::DestroyInstance();
		DebugClientManager::DestroyInstance();
		LicenceManager::DestroyInstance();
		MatchingManager::DestroyInstance();
		DataBaseManager::DestroyInstance();
		DBServerManager::DestroyInstance();
		TimerManager::DestroyInstance();

		delete Instance;
		Instance = nullptr;
	}
}
IOCPServer* MainManager::getServer()
{
	return server;
}

void MainManager::run()
{
	DBServerManager::GetInstance()->connectDBServer();

	server = new IOCPServer();
	if (server->start(SERVERPORT) == false)
	{
		printf("Failed to start Game Server.\n");
		return;
	}

	printf("Start Game Server\n");

	// 버퍼 크기 수정
	int size = server->getRecvBufferSize();
	server->setRecvBufferSize(size * 2);
	size = server->getSendBufferSize();
	server->setSendBufferSize(size * 2);

	while (true)
	{
		server->acceptClient(new User());
	}

	SocketInit::stop();
	delete server;
	printf("Shut down Game Server\n");
	system("pause");
}