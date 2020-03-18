#include "LicenceState.h"
#include "LicenceManager.h"

void LicenceState::recv(User* user)
{
	LicenceManager::GetInstance()->recv(user);
}
void LicenceState::disconnect(User* user)
{
	LicenceManager::GetInstance()->disconnect(user);
}
USER_STATE LicenceState::getStateType()
{
	return USER_STATE::LICENCE;
}