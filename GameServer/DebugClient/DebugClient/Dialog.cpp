#include "DialogManager.h"
#include "Dialog.h"

Dialog* Dialog::tempInstance = nullptr;

Dialog::Dialog(int dialogID)
{
	hWnd = nullptr;
	this->dialogID = dialogID;
}
INT_PTR CALLBACK Dialog::WndProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	Dialog* dialog = DialogManager::GetInstance()->getDialog(hDlg);

	switch(iMessage)
	{
	case WM_INITDIALOG:
		dialog = Dialog::tempInstance;
		dialog->hWnd = hDlg;
		DialogManager::GetInstance()->addDialog(dialog);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL)
		{
			if (dialog != nullptr)
			{
				DialogManager::GetInstance()->removeDialog(dialog);
			}
		}
		break;
	}

	if(dialog != nullptr)
	{
		return dialog->MessageProcess(iMessage, wParam, lParam);
	}
	return 0;
}
void Dialog::open(HWND parent)
{
	if (hWnd == nullptr)
	{
		Dialog::tempInstance = this;
		DialogBox(0, MAKEINTRESOURCE(dialogID), parent, WndProc);
	}
}
DWORD WINAPI Dialog::DialogThread(void* arg)
{
	ThreadData* data = (ThreadData*)arg;
	Dialog* dialog = data->dialog;
	HWND parent = data->parent;
	delete data;

	Dialog::tempInstance = dialog;
	DialogBox(0, MAKEINTRESOURCE(dialog->dialogID), parent, dialog->WndProc);
	return 0;
}
void Dialog::openOnNewThread(HWND parent)
{
	if (hWnd == nullptr)
	{
		ThreadData* data = new ThreadData();
		data->dialog = this;
		data->parent = parent;

		CreateThread(0, 0, Dialog::DialogThread, data, 0, 0);
	}
}
void Dialog::close()
{
	if (hWnd != nullptr)
	{
		EndDialog(hWnd, IDCANCEL);
		hWnd = nullptr;
	}
}
HWND Dialog::getHWND()
{
	return hWnd;
}

void Dialog::listbox_addItem(int id, const char* name, long long data, int insertIndex)
{
	HWND hList = GetDlgItem(hWnd, id);
	if (insertIndex == -1) {
		insertIndex = SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)name);
		SendMessage(hList, LB_SETITEMDATA, insertIndex, data);
	}
	else {
		SendMessage(hList, LB_INSERTSTRING, insertIndex, (LPARAM)name);
		SendMessage(hList, LB_SETITEMDATA, insertIndex, data);
	}
}
void Dialog::listbox_removeItem(int id, int index)
{
	HWND hList = GetDlgItem(hWnd, id);
	SendMessage(hList, LB_DELETESTRING, index, 0);
}
int Dialog::listbox_getSelect(int id)
{
	HWND hList = GetDlgItem(hWnd, id);
	return SendMessage(hList, LB_GETCURSEL, 0, 0);
}
void Dialog::listbox_setSelect(int id, int index)
{
	HWND hList = GetDlgItem(hWnd, id);
	SendMessage(hList, LB_SETCURSEL, index, 0);
}
int Dialog::listbox_getIndex(int id, const char* name)
{
	HWND hList = GetDlgItem(hWnd, id);
	return SendMessage(hList, LB_FINDSTRING, -1, (LPARAM)name);
}
void Dialog::listbox_removeItem(int id, const char* name)
{
	HWND hList = GetDlgItem(hWnd, id);
	int index = -1;
	while (true) {
		index = SendMessage(hList, LB_FINDSTRING, index, (LPARAM)name);
		if (index == -1)
		{
			break;
		}
		listbox_removeItem(id, index);
		index --;
	}
}
void Dialog::listbox_getItem(int id, int index, char* name)
{
	HWND hList = GetDlgItem(hWnd, id);
	SendMessage(hList, LB_GETTEXT, index, (LPARAM)name);
}

void Dialog::listbox_setData(int id, int index, long long data)
{
	HWND hList = GetDlgItem(hWnd, id);
	SendMessage(hList, LB_SETITEMDATA, index, data);
}
long long Dialog::listbox_getData(int id, int index)
{
	HWND hList = GetDlgItem(hWnd, id);
	return SendMessage(hList, LB_GETITEMDATA, index, 0);
}

int Dialog::listbox_getCount(int id)
{
	HWND hList = GetDlgItem(hWnd, id);
	return SendMessage(hList, LB_GETCOUNT, 0, 0);
}

void Dialog::text_addString(int id, const char* message)
{
	HWND txt = GetDlgItem(hWnd, id);
	int nLength = GetWindowTextLength(txt);
	SendMessage(txt, EM_SETSEL, nLength, nLength);
	SendMessage(txt, EM_REPLACESEL, FALSE, (LPARAM)message);
}
void Dialog::text_setString(int id, const char* message)
{
	SetDlgItemText(hWnd, id, message);
}
void Dialog::text_getString(int id, char* message, int maxSize)
{
	GetDlgItemText(hWnd, id, message, maxSize);
}
void Dialog::text_reset(int id)
{
	SetDlgItemText(hWnd, id, "");
}
bool Dialog::checkbox_getCheck(int id)
{
	HWND checkbox = GetDlgItem(hWnd, id);
	return SendMessage(checkbox, BM_GETCHECK, 0, 0);
}
void Dialog::checkbox_setCheck(int id, bool check)
{
	HWND checkbox = GetDlgItem(hWnd, id);
	SendMessage(checkbox, BM_SETCHECK, (WPARAM)check, 0);
}