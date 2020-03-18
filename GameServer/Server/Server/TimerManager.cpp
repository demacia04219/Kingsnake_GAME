#include "TimerManager.h"
#include <time.h>

TimerManager* TimerManager::Instance = nullptr;


TimerManager* TimerManager::CreateInstance()
{
	if (Instance == nullptr)
	{
		Instance = new TimerManager();
	}
	return Instance;
}
TimerManager* TimerManager::GetInstance()
{
	return Instance;
}
void TimerManager::DestroyInstance()
{
	if (Instance != nullptr)
	{
		delete Instance;
		Instance = nullptr;
	}
}

void TimerManager::addTimer(TimerProcess* timer, double milli_sec)
{
	if (milli_sec < 1) {
		throw "frameLate는 반드시 1 이상 값이어야 함.";
		return;
	}
	std::list<Timer*>::iterator i;

	EnterCriticalSection(&cs);
	for (i = list.begin(); i != list.end(); i++)
	{
		if ((*i)->millisec == milli_sec)
		{
			EnterCriticalSection(&(*i)->cs);
			(*i)->list.push_back(timer);
			LeaveCriticalSection(&(*i)->cs);
			LeaveCriticalSection(&cs);
			return;
		}
	}

	Timer* newTimer = new Timer();
	newTimer->millisec = milli_sec;
	newTimer->threadFlag = new bool; 
	(*newTimer->threadFlag) = true;
	InitializeCriticalSection(&newTimer->cs);
	newTimer->list.push_back(timer);

	HANDLE th = CreateThread(NULL, 0, TimerManager::tick, newTimer, 0, NULL);
	CloseHandle(th);
	list.push_back(newTimer);
	LeaveCriticalSection(&cs);
}
void TimerManager::removeTimer(TimerProcess* timer, double milli_sec)
{
	std::list<Timer*>::iterator i;

	EnterCriticalSection(&cs);
	for (i = list.begin(); i != list.end(); i++)
	{
		Timer* t = (*i);
		if (t->millisec == milli_sec)
		{
			EnterCriticalSection(&t->cs);
			t->list.remove(timer);
			if (t->list.empty())
			{
				list.remove(t);
				(*t->threadFlag) = false;
			}
			LeaveCriticalSection(&t->cs);
			LeaveCriticalSection(&cs);
			return;
		}
	}
	LeaveCriticalSection(&cs);
}
int TimerManager::getThreadCount()
{
	return list.size();
}
DWORD WINAPI TimerManager::tick(void* arg)
{
	Timer* timer = (Timer*)arg;
	bool* flag = timer->threadFlag;

	clock_t time = clock();
	unsigned long long count = 0;

	while (true)
	{
		long target = time + timer->millisec * count;
		long delta = target - clock();
		if (delta < 0) {
			delta = 0;
		}
		Sleep(delta);
		count++;


		EnterCriticalSection(&timer->cs);
		std::list<TimerProcess*> list = timer->list;// 임시 복사. 안전한 접근을 위해서.
		for (TimerProcess* data : list)
		{
			data->process();
			if (*flag == false)
				break;
		}

		if (*flag == false) {
			LeaveCriticalSection(&timer->cs);
			break;
		}

		LeaveCriticalSection(&timer->cs);
	}

	delete flag;
	DeleteCriticalSection(&timer->cs);
	delete timer;
	return 0;
}