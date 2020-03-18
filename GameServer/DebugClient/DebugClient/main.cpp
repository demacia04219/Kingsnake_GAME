#include "MainManager.h"
#include <Windows.h>

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	MainManager::CreateInstance();
	MainManager::GetInstance()->run();
	MainManager::DestroyInstance();
	return 0;
}