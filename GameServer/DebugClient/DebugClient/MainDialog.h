#ifndef __MAIN_DIALOG_H__
#define __MAIN_DIALOG_H__

#include "Dialog.h"
#include "resource.h"

class Client;
class Matching;

class MainDialog : public Dialog
{
private:
	void setListName(Client* client, char* name);
	void setMatchingName(Matching* matching, char* name);
	void selectMatchingOfSelectedClient();
	void selectClientOfSelectedMatching();

public:
	MainDialog() : Dialog(DIALOG_MAIN) {};
	BOOL MessageProcess(UINT iMessage, WPARAM wParam, LPARAM lParam) override;
};

#endif