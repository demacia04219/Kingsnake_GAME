#include "DialogManager.h"
#include "Dialog.h"

DialogManager* DialogManager::Instance = nullptr;

Dialog* DialogManager::getDialog(HWND hWnd)
{
	for (list<Dialog*>::iterator i = dialogList.begin(); i != dialogList.end(); ++i)
	{
		Dialog* data = (*i);
		if (data->hWnd == hWnd)
		{
			return data;
		}
	}
	return nullptr;
}


DialogManager* DialogManager::CreateInstance()
{
	if (Instance == nullptr)
	{
		Instance = new DialogManager();
	}
	return Instance;
}
DialogManager* DialogManager::GetInstance()
{
	return Instance;
}
void DialogManager::DestroyInstance()
{
	if (Instance != nullptr)
	{
		delete Instance;
		Instance = nullptr;
	}
}
void DialogManager::addDialog(Dialog* dialog)
{
	dialogList.push_back(dialog);
}
void DialogManager::removeDialog(Dialog* dialog)
{
	dialogList.remove(dialog);
}