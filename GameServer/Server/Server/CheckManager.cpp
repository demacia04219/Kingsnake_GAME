#include "User.h"
#include "Player.h"
#include "DebugClientManager.h"
#include "CheckManager.h"
#include "Matching.h"
#include <time.h>
#include "DBServerManager.h"
#include "CheckState.h"
#include "DataBase.h"
#include "Encoding.h"

CheckManager* CheckManager::Instance = nullptr;

CheckManager* CheckManager::CreateInstance()
{
	if (Instance == nullptr)
	{
		Instance = new CheckManager();
	}
	return Instance;
}

CheckManager* CheckManager::GetInstance()
{
	return Instance;
}
void CheckManager::DestroyInstance()
{
	if (Instance != nullptr)
	{
		delete Instance;
		Instance = nullptr;
	}
}
void CheckManager::recv(User* user)
{
	// 프로토콜 추출											
	GAME_PROTOCOL protocol;
	*user >> protocol;

	// 프로토콜 판별
	switch (protocol)
	{
	case GAME_PROTOCOL::CLIENT_PONG:
		recvPong(*user);
		break;
	case GAME_PROTOCOL::CLIENT_GOOGLE_JOIN_CHECK:
		recvGoogleJoinCheck(*user);
		break;
	case GAME_PROTOCOL::CLIENT_GOOGLE_JOIN:
		recvGoogleJoin(*user);
		break;
	case GAME_PROTOCOL::CLIENT_GOOGLE_LOGIN:
		recvGoogleLogin(*user);
		break;
	}
}
void CheckManager::disconnect(User* user)
{

}
void CheckManager::recvPong(User& user)
{
	user.setPong();
}
void CheckManager::recvGoogleJoinCheck(User& user)
{
	std::string id;
	wchar_t wid[50];
	char *aid;
	user >> wid;
	Encoding::UnicodeToAscii(wid, aid);
	id = aid;
	id = "1" + id;
	delete aid;

	user.EnterCS();
	LoginLicence* licence = user.getLoginLicence();
	if (licence == nullptr) {
		user.LeaveCS();
		return;
	}
	licence->id = id;
	licence->isLoginReq = false;

	if (user.getIsLogin() == false) {
		user.getUserData().id = id;

		DBServerManager::GetInstance()->sendCheckUserExist(user);
	}
	user.LeaveCS();
}
void CheckManager::recvGoogleJoin(User& user)
{
	std::string id, nick;
	wchar_t wid[50], wnick[50];
	char *aid, *anick;
	user >> wid >> wnick;
	Encoding::UnicodeToAscii(wid, aid);
	Encoding::UnicodeToAscii(wnick, anick);
	id = aid;
	nick = anick;
	id = "1" + id;
	delete aid;
	delete anick;



	user.EnterCS();

	if (user.getIsLogin() == false) {
		user.getUserData().id = id;
		user.getUserData().nick = nick;

		DBServerManager::GetInstance()->sendRegister(user);
	}
	user.LeaveCS();
}
void CheckManager::recvGoogleLogin(User& user)
{
	std::string id;
	wchar_t wid[50];
	char *aid;
	user >> wid;
	Encoding::UnicodeToAscii(wid, aid);
	id = aid;
	id = "1" + id;
	delete aid;

	// 이미 누군가 로그인?
	if (DataBaseManager::GetInstance()->isLoggedIn(id)) {
		SendPacket sp;
		sp << SERVER_GOOGLE_LOGIN << false;
		user.sendMsg(sp);
		return;
	}

	user.EnterCS();
	LoginLicence* licence = user.getLoginLicence();
	if (licence == nullptr) {
		user.LeaveCS();
		return;
	}
	licence->id = id;
	licence->isLoginReq = true;

	if (user.getIsLogin() == false) {
		user.getUserData().id = id;

		DBServerManager::GetInstance()->sendCheckUserExist(user);
	}
	user.LeaveCS();
}