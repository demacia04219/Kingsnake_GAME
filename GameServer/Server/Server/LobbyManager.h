#ifndef __LOBBY_MANAGER_H__
#define __LOBBY_MANAGER_H__

class User;

class LobbyManager
{
private:
	static LobbyManager* Instance;

	LobbyManager() {}
	~LobbyManager() {}

	void recvPong(User& user);
	void recvSetSubCharacter(User& user);
	void recvSetItemSkill(User& user);
	void recvSetCostume(User& user);
	void recvReqMatching(User& user);

public:
	static LobbyManager* CreateInstance();
	static LobbyManager* GetInstance();
	static void DestroyInstance();

	void recv(User* user);
	void disconnect(User* user);
};

#endif