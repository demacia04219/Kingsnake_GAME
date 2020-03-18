#include "MainManager.h"
#include "Global.h"

int main()
{
	printf("Game Server v%s\n\n", SERVER_VERSION);

	MainManager::CreateInstance();
	MainManager::GetInstance()->run();
	MainManager::DestroyInstance();
	return 0;
}