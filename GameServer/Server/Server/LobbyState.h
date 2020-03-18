#ifndef __MAIN_STATE_H__
#define __MAIN_STATE_H__
#include "State.h"

class User;

// 클라 버전, 내장 데이터 버전, 로그인, 중복 접속 체크
class LobbyState : public State
{
public:
	LobbyState() {}
	~LobbyState() {}
	void recv(User* user) override;
	void disconnect(User* user) override;
	USER_STATE getStateType() override;
};

#endif