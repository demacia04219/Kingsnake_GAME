#ifndef __MATCHING_STATE_H__
#define __MATCHING_STATE_H__

#include "State.h"

class User;
class Matching;

class MatchingState : public State
{
private:
	Matching * matching = nullptr;

public:
	MatchingState(){}
	~MatchingState() {}
	void recv(User* user) override;
	void disconnect(User* user) override;
	USER_STATE getStateType() override;

	Matching* getMatching();
	void setMatching(Matching* matching);
};

#endif