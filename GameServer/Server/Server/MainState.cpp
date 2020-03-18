#include "LobbyState.h"
#include "LobbyManager.h"

void LobbyState::recv(User* user) {
	LobbyManager::GetInstance()->recv(user);
}
void LobbyState::disconnect(User* user) {
	LobbyManager::GetInstance()->disconnect(user);
}
USER_STATE LobbyState::getStateType() {
	return USER_STATE::LOBBY;
}