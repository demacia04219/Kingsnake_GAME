#ifndef __DIALOG_MANAGER_H__
#define __DIALOG_MANAGER_H__

#include <Windows.h>
#include <list>
using namespace std;

class Dialog;

class DialogManager
{
private:
	static DialogManager * Instance;
	list<Dialog*> dialogList;

	DialogManager() {}
	~DialogManager() {}

public:
	static DialogManager* CreateInstance();
	static DialogManager* GetInstance();
	static void DestroyInstance();

	Dialog * getDialog(HWND hWnd);
	void addDialog(Dialog* dialog);
	void removeDialog(Dialog* dialog);
};

#endif