#ifndef __CHECK_STATE_H__
#define __CHECK_STATE_H__
#include "State.h"
#include <string>

class User;
struct LoginLicence
{
	std::string id = "";
	bool isLoginReq = false;
};

// Ŭ�� ����, ���� ������ ����, �α���, �ߺ� ���� üũ
class CheckState : public State
{
public:
	LoginLicence licence;

	CheckState() {}
	~CheckState() {}
	void recv(User* user) override;
	void disconnect(User* user) override;
	USER_STATE getStateType() override;
};

#endif