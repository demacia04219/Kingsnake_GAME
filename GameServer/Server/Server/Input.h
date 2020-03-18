#ifndef __INPUT_H__
#define __INPUT_H__

#include <Windows.h>

class Input
{
private:
	static Input* instance;
	bool* flag;

	Input();
	~Input();

	static DWORD WINAPI process(void*);
public:
	static Input* GetInstance();
	static Input* CreateInstance();
	static void DestroyInstance();
};

#endif