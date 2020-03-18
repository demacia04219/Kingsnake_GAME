#include "LicenceManager.h"
#include "Global.h"
#include "User.h"
#include "GlobalProtocol.h"
#include "DataBase.h"
#include "MatchingManager.h"
#include "NetworkException.h"

#include <stdio.h>

LicenceManager* LicenceManager::Instance = nullptr;

LicenceManager* LicenceManager::CreateInstance()
{
	if (Instance == nullptr)
	{
		Instance = new LicenceManager();
	}
	return Instance;
}
LicenceManager* LicenceManager::GetInstance()
{
	return Instance;
}
void LicenceManager::DestroyInstance()
{
	if (Instance != nullptr)
	{
		delete Instance;
		Instance = nullptr;
	}
}

void LicenceManager::recv(User* user)
{
	LICENCE_PROTOCOL protocol;
	if(user->canGetData(sizeof(LICENCE_PROTOCOL)))
		*user >> protocol;
	else {
		printf("Unknown client sent a packet. So, that packet was blocked.\n");
	}

	switch (protocol)
	{
	case LICENCE_PROTOCOL::CLIENT_PASSWORD:
		checkPassword(user);
		break;
	default:
		printf("Unknown client sent a packet. So, that packet was blocked.\n");
		break;
	}
}
void LicenceManager::checkPassword(User* user)
{
	int password[4] = { 0 };
	int debug[4] = PASSWORD_DEBUG;	// 디버그 클라 인증 비밀번호
	int game[4] = PASSWORD_GAME;	// 게임 클라 인증 비밀번호

	try {
		user->getData(password, 16);
	}
	catch (PacketException* exception)
	{
		printf("Unknown client sent a packet. So, that packet was blocked.\n");
		return;
	}
	
	bool check = true;
	for (int i = 0; i < 4; i++) {
		if (password[i] != game[i]){
			check = false;
			break;
		}
	}
	if (check) {
		SendPacket sp;
		sp << LICENCE_PROTOCOL::SERVER_RESULT << check;
		user->sendMsg(sp);
		user->setState(GAME_INTRO_STATE);

		// 매칭 테스트를 위해 임시로 넣은 코드
		if(DEBUG_MATCHING)
			MatchingManager::GetInstance()->matching(user);
		//
		printf("Client %d accessed as the game client.\n", user->getSerial());
		return;
	}

	check = true;
	for (int i = 0; i < 4; i++) {
		if (password[i] != debug[i]) {
			check = false;
			break;
		}
	}
	if (check) {
		user->setState(DEBUG_INTRO_STATE);
		user->setNagle(false);// 디버깅 클라 네이글 해제

		SendPacket sp;
		sp << LICENCE_PROTOCOL::SERVER_RESULT << check;
		user->sendMsg(sp);
		DataBaseManager::GetInstance()->addDebugUser(user);
		printf("Client %d accessed as the debug client.\n", user->getSerial());
		return;
	}

	SendPacket sp;
	sp << LICENCE_PROTOCOL::SERVER_RESULT << check;
	user->sendMsg(sp);
	printf("Client %d sent an invalid password.\n", user->getSerial());
}
void LicenceManager::disconnect(User* user)
{

}