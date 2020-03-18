#ifndef __LICENCE_STATE_H__
#define __LICENCE_STATE_H__

#include "State.h"

class User;

class LicenceState : public State
{
public:
	LicenceState() {}
	~LicenceState() {}
	void recv(User* user) override;
	void disconnect(User* user) override;
	USER_STATE getStateType() override;
};

#endif