#ifndef __DB_SERVER_MANAGER_H__
#define __DB_SERVER_MANAGER_H__

#include "TCPClient.h"
#include "UserData.h"

class User;

class DBServerManager
{
private:
	static DBServerManager* Instance;
	TCPClient& client;
	bool connected;

	static DWORD WINAPI recvProcess(void* arg);

	DBServerManager();
	~DBServerManager();

public:

	static DBServerManager* CreateInstance();
	static DBServerManager* GetInstance();
	static void DestroyInstance();

	void connectDBServer();
	bool isConnected();
	TCPClient& getSocket();

	/* ���̺� ���� ��� */
	void sendSetBasicInfo(User&);
	void sendSetMatchInfo(User&);
	void sendSetCharacterDack(User&);
	void sendSetItemDack(User&);
	void sendSetEmotionDack(User&);
	void sendSetCharacterInventory(User&);
	void sendSetItemInventory(User&);
	void sendSetCostumeInventory(User&);
	void sendSetEmotionInventory(User&);

	void sendBasicInfo(User&);
	void sendMatchInfo(User&);
	void sendCharacterDack(User&);
	void sendItemDack(User&);
	void sendEmotionDack(User&);
	void sendItemInventory(User&);
	void sendCostumeInventory(User&);
	void sendEmotionInventory(User&);
	void sendCharacterInventory(User&);
	void sendReq(User& user, UserData::USER_DATA userData);// ��� ������ ��û

	/* ���� ���� ���� ���� üũ */
	void sendCheckUserExist(User&);

	/* ���� & Ż��*/
	void sendRegister(User&);
	void sendWithDraw(User&);

private:

	/* ���̺� ���� ��� */
	void recvSetBasicInfo();
	void recvSetMatchInfo();
	void recvSetCharacterDack();
	void recvSetItemDack();
	void recvSetEmotionDack();
	void recvSetCharacterInventory();
	void recvSetItemInventory();
	void recvSetCostumeInventory();
	void recvSetEmotionInventory();

	/* ���̺� Ž�� ��� */
	void recvBasicInfo();
	void recvMatchInfo();
	void recvCharacterDack();
	void recvItemDack();
	void recvEmotionDack();
	void recvCharacterInventory();
	void recvItemInventory();
	void recvCostumeInventory();
	void recvEmotionInventory();

	/* ���� ���� ���� ���� üũ */
	void recvCheckUserExist();

	/* ���� & Ż��*/
	void recvRegister();
	void recvWithDraw();
	
};

#endif