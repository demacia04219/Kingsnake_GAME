#include "CheckState.h"
#include "CheckManager.h"

void CheckState::recv(User* user) {
	CheckManager::GetInstance()->recv(user);
}
void CheckState::disconnect(User* user) {
	CheckManager::GetInstance()->disconnect(user);
}
USER_STATE CheckState::getStateType() {
	return USER_STATE::CHECK;
}