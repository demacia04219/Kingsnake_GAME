#ifndef __STATE_H__
#define __STATE_H__

#include "Global.h"

class User;

// 스테이트 패턴

class State abstract
{
private:
public:
	virtual USER_STATE getStateType() = 0;
	virtual void recv(User* user) = 0;
	virtual void disconnect(User* user) = 0;
};
#endif