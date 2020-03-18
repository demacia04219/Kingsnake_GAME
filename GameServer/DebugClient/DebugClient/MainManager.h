#ifndef __MAIN_MANAGER_H__
#define __MAIN_MANAGER_H__

#include <Windows.h>

class MainManager
{
private:
	static MainManager* Instance;
	CRITICAL_SECTION cs;

	MainManager() { InitializeCriticalSection(&cs); }
	~MainManager() { DeleteCriticalSection(&cs); }

public:
	static MainManager* CreateInstance();
	static MainManager* GetInstance();
	static void DestroyInstance();

	void run();
	void EnterCS();
	void LeaveCS();
};

#endif