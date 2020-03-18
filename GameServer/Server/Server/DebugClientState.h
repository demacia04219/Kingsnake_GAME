#ifndef __DEBUG_MAIN_STATE_H__
#define __DEBUG_MAIN_STATE_H__

#include "State.h"
#include <vector>

class DebugClientState : public State
{
public:
	std::vector<int> matchingList;

	DebugClientState() {}
	~DebugClientState() {}
	void recv(User* user) override;
	void disconnect(User* user) override;
	USER_STATE getStateType() override;
};
#endif