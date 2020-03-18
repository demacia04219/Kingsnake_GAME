#ifndef __MAIN_STATE_H__
#define __MAIN_STATE_H__
#include "State.h"

class User;

// Ŭ�� ����, ���� ������ ����, �α���, �ߺ� ���� üũ
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