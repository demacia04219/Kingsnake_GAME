#ifndef __TIMER_MANAGER_H__
#define __TIMER_MANAGER_H__

#include <Windows.h>
#include <list>

class TimerProcess abstract
{
public:
	virtual void process() = 0;
};

class TimerManager
{
private:
	struct Timer
	{
		bool* threadFlag;
		double millisec;	// tick 시간(밀리 초)
		std::list<TimerProcess*> list;
		CRITICAL_SECTION cs;
	};

	CRITICAL_SECTION cs;

	static TimerManager* Instance;
	std::list<Timer*> list;

	TimerManager() { InitializeCriticalSection(&cs); }
	~TimerManager() { DeleteCriticalSection(&cs); }

public:
	static TimerManager* CreateInstance();
	static TimerManager* GetInstance();
	static void DestroyInstance();

	void addTimer(TimerProcess* timer, double milli_sec);
	void removeTimer(TimerProcess* timer, double milli_sec);
	int getThreadCount();
	
	static DWORD WINAPI tick(void* arg);
};

#endif