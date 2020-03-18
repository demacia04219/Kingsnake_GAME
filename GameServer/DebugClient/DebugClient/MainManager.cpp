#include "NetworkManager.h"
#include "DialogManager.h"
#include "ProjectManager.h"
#include "MainManager.h"

MainManager* MainManager::Instance = nullptr;

MainManager* MainManager::CreateInstance()
{
	if (Instance == nullptr)
	{
		Instance = new MainManager();

		DialogManager::CreateInstance();
		ProjectManager::CreateInstance();
		NetworkManager::CreateInstance();
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
		NetworkManager::DestroyInstance();
		ProjectManager::DestroyInstance();
		DialogManager::DestroyInstance();

		delete Instance;
		Instance = nullptr;
	}
}

void MainManager::run()
{
	if (NetworkManager::GetInstance()->connect() == false)
	{
		MessageBox(0, "서버 연결 실패", "Message", MB_OK);
		return;
	}

	ProjectManager::GetInstance()->run();
}
void MainManager::EnterCS()
{
	EnterCriticalSection(&cs);
}
void MainManager::LeaveCS() {
	LeaveCriticalSection(&cs);
}