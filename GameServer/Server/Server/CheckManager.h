#ifndef __CHECK_MANAGER_H__
#define __CHECK_MANAGER_H__

class User;

class CheckManager
{
private:
	static CheckManager* Instance;

	CheckManager() {}
	~CheckManager() {}

	void recvPong(User& user);
	void recvGoogleJoinCheck(User& user);
	void recvGoogleJoin(User& user);
	void recvGoogleLogin(User& user);

public:
	static CheckManager* CreateInstance();
	static CheckManager* GetInstance();
	static void DestroyInstance();
	
	void recv(User* user);
	void disconnect(User* user);
};

#endif