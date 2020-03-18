#include "NetworkManager.h"
#include "MainDialog.h"
#include "ProjectManager.h"
#include "MainManager.h"

BOOL MainDialog::MessageProcess(UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	MainManager* main = MainManager::GetInstance();

	switch (iMessage)
	{
	case WM_INITDIALOG:
		ProjectManager::GetInstance()->wakeUp();
		return 0;
	case WM_COMMAND:

		switch(LOWORD(wParam)){
		case MAIN_LIST1:
			if(HIWORD(wParam) == LBN_SELCHANGE)
				selectMatchingOfSelectedClient();
			return 0;
		case MAIN_LIST2:
			if (HIWORD(wParam) == LBN_SELCHANGE)
				selectClientOfSelectedMatching();
			return 0;
		case IDCANCEL:
			close();
			return 0;
		case MAIN_BTN1:
		{
			int index = listbox_getSelect(MAIN_LIST2);
			if (index != -1) {
				Matching* m = (Matching*)(unsigned long long)listbox_getData(MAIN_LIST2, index);
				if(ProjectManager::GetInstance()->isAlreadyEnterMatching(m->serial) == false)
					NetworkManager::GetInstance()->sendEnterMatching(m->serial);
			}
		}
			return 0;
		}

		return 0;
	case MESSAGE::CONNECT:
	{
		char msg[150];
		main->EnterCS();
		sprintf(msg, "Connected Client (%d)", NetworkManager::GetInstance()->getClientCount());
		text_setString(MAIN_TXT1, msg);
		int count = listbox_getCount(MAIN_LIST1);
		bool did = false;
		for (int i = 0; i < count; i++)
		{
			long long data = listbox_getData(MAIN_LIST1, i);
			if (data == (long long)wParam) {
				did = true;
				Client* c = (Client*)(unsigned long long)data;
				char msg[150];
				setListName(c, msg);
				listbox_removeItem(MAIN_LIST1, i);
				listbox_addItem(MAIN_LIST1, msg, (long long)c, i);
				break;
			}
		}
		if (did == false) {
			char msg[150];
			setListName((Client*)wParam, msg);
			listbox_addItem(MAIN_LIST1, msg, wParam);
		}
		main->LeaveCS();
	}
		return 0;
	case MESSAGE::CREATE_MATCHING:
	{
		char msg[100];
		main->EnterCS();
		sprintf(msg, "Ongoing Matching (%d)", NetworkManager::GetInstance()->getMatchingCount());
		text_setString(MAIN_TXT2, msg);
		int count = listbox_getCount(MAIN_LIST2);
		bool did = false;
		for (int i = 0; i < count; i++)
		{
			long long data = listbox_getData(MAIN_LIST2, i);
			if (data == (long long)wParam) {
				did = true;
				Matching* c = (Matching*)(unsigned long long)data;
				char msg[100];
				setMatchingName(c, msg);
				listbox_removeItem(MAIN_LIST2, i);
				listbox_addItem(MAIN_LIST2, msg, (long long)c, i);
				break;
			}
		}
		if (did == false) {
			char msg[100];
			setMatchingName((Matching*)wParam, msg);
			listbox_addItem(MAIN_LIST2, msg, wParam);
		}
		main->LeaveCS();
	}
		return 0;
	case MESSAGE::DISCONNECT:
	{
		char msg[100];
		main->EnterCS();
		sprintf(msg, "Connected Client (%d)", NetworkManager::GetInstance()->getClientCount());
		text_setString(MAIN_TXT1, msg);
		int count = listbox_getCount(MAIN_LIST1);
		for (int i = 0; i < count; i++)
		{
			int data = (WPARAM)listbox_getData(MAIN_LIST1, i);
			if (data == (int)wParam) {
				listbox_removeItem(MAIN_LIST1, i);
				delete (Client*)wParam;
			}
		}
		main->LeaveCS();
	}
		return 0;
	case MESSAGE::DESTROY_MATCHING: 
	{
		char msg[100];
		main->EnterCS();
		sprintf(msg, "Ongoing Matching (%d)", NetworkManager::GetInstance()->getMatchingCount());
		text_setString(MAIN_TXT2, msg);
		int count = listbox_getCount(MAIN_LIST2);
		for (int i = 0; i < count; i++)
		{
			int data = (WPARAM)listbox_getData(MAIN_LIST2, i);
			if (data == (int)wParam) {
				listbox_removeItem(MAIN_LIST2, i);
				delete (Matching*)wParam;
			}
		}
		main->LeaveCS();
	}
		return 0;
	case MESSAGE::USERLIST:
	{
		main->EnterCS();
		NetworkManager* manager = NetworkManager::GetInstance();
		int count = manager->getClientCount();
		char msg[100];
		sprintf(msg, "Connected Client (%d)", count);
		text_setString(MAIN_TXT1, msg);
		for (int i = 0; i < count; i++)
		{
			Client* c = manager->getClient(i);
			char msg[100];
			setListName(c, msg);
			listbox_addItem(MAIN_LIST1, msg, (long long)c);
		}
		main->LeaveCS();
	}
		return 0;
	case MESSAGE::MATCHINGLIST:
		main->EnterCS();
		NetworkManager * manager = NetworkManager::GetInstance();
		int count = manager->getMatchingCount();
		char msg[100];
		sprintf(msg, "Ongoing Matching (%d)", count);
		text_setString(MAIN_TXT2, msg);
		for (int i = 0; i < count; i++)
		{
			Matching* c = manager->getMatching(i);
			char msg[100];
			setMatchingName(c, msg);
			listbox_addItem(MAIN_LIST2, msg, (long long)c);
		}
		main->LeaveCS();
		return 0;
	}
	return 0;
}
void MainDialog::setListName(Client* client, char* name)
{
	char stat[30];
	char me[20] = {0};
	switch (client->state)
	{
	case USER_STATE::DEBUG_MAIN:
		strcpy(stat, "debug client");
		break;
	case USER_STATE::MATCHING:
		strcpy(stat, "matching");
		break;
	case USER_STATE::LICENCE:
		strcpy(stat, "invalid");
		break;
	case USER_STATE::CHECK:
		strcpy(stat, "checking");
		break;
	case USER_STATE::MAIN:
		strcpy(stat, "lobby");
		break;
	}
	if (client->serial == NetworkManager::GetInstance()->getSerial()) {
		strcpy(me, "³ª");
	}
	else {
		sprintf(me, "Client %d", client->serial);
	}
	sprintf(name, "[ %s ] %s (%s)", me, client->ip, stat);
}
void MainDialog::setMatchingName(Matching* matching, char* name)
{
	sprintf(name, "[ Matching %d ] Client %d vs Client %d", matching->serial, matching->player1, matching->player2);
}
void MainDialog::selectMatchingOfSelectedClient()
{
	int index = listbox_getSelect(MAIN_LIST1);
	if (index == -1)
	{
		return;
	}
	Client* c = (Client*)(unsigned long long)listbox_getData(MAIN_LIST1, index);
	int serial = c->serial;

	int count = listbox_getCount(MAIN_LIST2);
	listbox_setSelect(MAIN_LIST2, -1);
	for (int i = 0; i < count; i++)
	{
		Matching* m = (Matching*)(unsigned long long)listbox_getData(MAIN_LIST2, i);

		if (m->player1 == serial || m->player2 == serial)
		{
			listbox_setSelect(MAIN_LIST2, i);
			break;
		}
	}
}

void MainDialog::selectClientOfSelectedMatching()
{
	int count;
	int index = listbox_getSelect(MAIN_LIST2);
	if (index == -1)
	{
		return;
	}
	Matching* m = (Matching*)(unsigned long long)listbox_getData(MAIN_LIST2, index);
	
	index = listbox_getSelect(MAIN_LIST1);
	Client* c = (Client*)(unsigned long long)listbox_getData(MAIN_LIST1, index);

	if (index != -1 && m->player1 == c->serial) {
		count = listbox_getCount(MAIN_LIST1);
		for (int i = index+1; i < count; i++) {
			c = (Client*)(unsigned long long)listbox_getData(MAIN_LIST1, i);
			if (m->player1 == c->serial || m->player2 == c->serial) {
				listbox_setSelect(MAIN_LIST1, i);
				return;
			}
		}
	}
	else {
		count = listbox_getCount(MAIN_LIST1);
		for (int i = 0; i < count; i++) {
			c = (Client*)(unsigned long long)listbox_getData(MAIN_LIST1, i);
			if (m->player1 == c->serial || m->player2 == c->serial) {
				listbox_setSelect(MAIN_LIST1, i);
				return;
			}
		}
	}
}