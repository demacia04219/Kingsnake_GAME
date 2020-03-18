#ifndef __PROJECT_MANAGER_H__
#define __PROJECT_MANAGER_H__

#include <vector>
#include <Windows.h>
class MainDialog;
class MatchingDialog;

class ProjectManager
{
private:
	static ProjectManager* Instance;
	std::vector<MatchingDialog*> matchingList;
	MainDialog* mainDialog;
	HANDLE event;

	ProjectManager();
	~ProjectManager();
public:
	static ProjectManager* CreateInstance();
	static ProjectManager* GetInstance();
	static void DestroyInstance();

	void run();
	void waitRecv();
	void wakeUp();
	HWND getMainHWND();
	MainDialog* getMainDialog();

	void addMatchingDialog(MatchingDialog* dialog);
	void removeMatchingDialog(MatchingDialog* dialog);
	int getMatchingDialogCount();
	MatchingDialog* getMatchingDialog(int index);
	bool isAlreadyEnterMatching(int matching_serial);
};
#endif