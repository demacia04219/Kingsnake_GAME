#ifndef __DIALOG_H__
#define	__DIALOG_H__

#include <Windows.h>

class DialogManager;

class Dialog
{
	friend class DialogManager;
	struct ThreadData {
		Dialog* dialog;
		HWND parent;
	};

private:
	int dialogID;
	static Dialog* tempInstance;
	static INT_PTR CALLBACK WndProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);
	static DWORD WINAPI DialogThread(void* arg);

protected:
	HWND hWnd;
	virtual BOOL MessageProcess(UINT iMessage, WPARAM wParam, LPARAM lParam) = 0;

public:
	Dialog(int dialogID);
	void open(HWND parent=0);// 인수 디폴트 값은 모달리스, 부모 HWND를 인수로 받으면 모달
	void openOnNewThread(HWND parent = 0);// 인수 디폴트 값은 모달리스, 부모 HWND를 인수로 받으면 모달
	void close();
	HWND getHWND();

	void listbox_addItem(int id, const char* name, long long data = 0, int insertIndex = -1);
	void listbox_getItem(int id, int index, char* name);
	void listbox_removeItem(int id, const char* name);
	void listbox_removeItem(int id, int index);
	int listbox_getIndex(int id, const char* name);
	int listbox_getSelect(int id);
	void listbox_setSelect(int id, int index);
	int listbox_getCount(int id);
	void listbox_setData(int id, int index, long long data);
	long long listbox_getData(int id, int index);

	void text_addString(int id, const char* message);
	void text_setString(int id, const char* message);
	void text_getString(int id, char* message, int maxSize);
	void text_reset(int id);

	bool checkbox_getCheck(int id);
	void checkbox_setCheck(int id, bool check);
};


#endif