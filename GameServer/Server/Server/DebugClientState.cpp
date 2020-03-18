#include "DebugClientState.h"
#include "DebugClientManager.h"

void DebugClientState::recv(User* user)
{
	DebugClientManager::GetInstance()->recv(user);
}
void DebugClientState::disconnect(User* user)
{
	DebugClientManager::GetInstance()->disconnect(user);
}
USER_STATE DebugClientState::getStateType()
{
	return USER_STATE::DEBUG;
}