#include "MatchingState.h"
#include "MatchingManager.h"

void MatchingState::recv(User* user)
{
	if (user->getMatching() != nullptr)
		user->getMatching()->ProtectDelete(user, false);

	MatchingManager::GetInstance()->recv(user);

	if (user->getMatching() != nullptr)
		user->getMatching()->ProtectDelete(user, true);
}
void MatchingState::disconnect(User* user)
{
	MatchingManager::GetInstance()->disconnect(user);
}
USER_STATE MatchingState::getStateType()
{
	return USER_STATE::MATCHING;
}
Matching* MatchingState::getMatching()
{
	return this->matching;
}
void MatchingState::setMatching(Matching* matching)
{
	this->matching = matching;
}
