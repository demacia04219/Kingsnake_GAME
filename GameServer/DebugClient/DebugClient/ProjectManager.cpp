#include "ProjectManager.h"
#include "Dialog.h"
#include "MainDialog.h"
#include "MatchingDialog.h"

ProjectManager* ProjectManager::Instance = nullptr;

ProjectManager::ProjectManager()
{
	mainDialog = new MainDialog();
	event = CreateEvent(0, FALSE, FALSE, 0);
}
ProjectManager::~ProjectManager()
{

}
ProjectManager* ProjectManager::CreateInstance()
{
	if (Instance == nullptr)
	{
		Instance = new ProjectManager();
	}
	return Instance;
}
ProjectManager* ProjectManager::GetInstance()
{
	return Instance;
}
void ProjectManager::DestroyInstance()
{
	if (Instance != nullptr)
	{
		delete Instance;
		Instance = nullptr;
	}
}

void ProjectManager::run()
{
	mainDialog->open();
}
void ProjectManager::waitRecv()
{
	WaitForSingleObject(event, INFINITE);
}
void ProjectManager::wakeUp()
{
	SetEvent(event);
}
HWND ProjectManager::getMainHWND()
{
	return mainDialog->getHWND();
}
MainDialog* ProjectManager::getMainDialog()
{
	return mainDialog;
}

void ProjectManager::addMatchingDialog(MatchingDialog* dialog)
{
	matchingList.push_back(dialog);
}
void ProjectManager::removeMatchingDialog(MatchingDialog* dialog)
{
	int count = matchingList.size();
	for (int i = 0; i < count; i++) {
		if (matchingList[i] == dialog) {
			matchingList.erase(matchingList.begin() + i);
			break;
		}
	}
}
int ProjectManager::getMatchingDialogCount()
{
	return matchingList.size();
}
MatchingDialog* ProjectManager::getMatchingDialog(int index)
{
	return matchingList[index];
}
bool ProjectManager::isAlreadyEnterMatching(int matching_serial)
{
	int count = matchingList.size();
	for (int i = 0; i < count; i++) {
		if (matchingList[i]->getMatchingInfo()->serial == matching_serial)
		{
			return true;
		}
	}
	return false;
}