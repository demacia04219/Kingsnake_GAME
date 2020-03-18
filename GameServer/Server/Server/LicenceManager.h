#ifndef __LICENCE_MANAGER_H__
#define __LICENCE_MANAGER_H__

class User;

class LicenceManager
{
private:
	static LicenceManager* Instance;

	LicenceManager() {}
	~LicenceManager() {}

public:
	static LicenceManager* CreateInstance();
	static LicenceManager* GetInstance();
	static void DestroyInstance();

	void recv(User* user);
	void disconnect(User* user);

	void checkPassword(User* user);
};

#endif