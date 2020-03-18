#ifndef __MAIN_MANAGER_H__
#define __MAIN_MANAGER_H__

class IOCPServer;

class MainManager
{
private:
	static MainManager* Instance;
	IOCPServer* server;

	MainManager();
	~MainManager();
public:
	static MainManager* CreateInstance();
	static MainManager* GetInstance();
	static void DestroyInstance();

	void run();
	IOCPServer* getServer();
};

#endif