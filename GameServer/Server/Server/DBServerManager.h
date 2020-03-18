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

	/* 테이블 변경 결과 */
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
	void sendReq(User& user, UserData::USER_DATA userData);// 모든 데이터 요청

	/* 유저 정보 존재 여부 체크 */
	void sendCheckUserExist(User&);

	/* 가입 & 탈퇴*/
	void sendRegister(User&);
	void sendWithDraw(User&);

private:

	/* 테이블 변경 결과 */
	void recvSetBasicInfo();
	void recvSetMatchInfo();
	void recvSetCharacterDack();
	void recvSetItemDack();
	void recvSetEmotionDack();
	void recvSetCharacterInventory();
	void recvSetItemInventory();
	void recvSetCostumeInventory();
	void recvSetEmotionInventory();

	/* 테이블 탐색 결과 */
	void recvBasicInfo();
	void recvMatchInfo();
	void recvCharacterDack();
	void recvItemDack();
	void recvEmotionDack();
	void recvCharacterInventory();
	void recvItemInventory();
	void recvCostumeInventory();
	void recvEmotionInventory();

	/* 유저 정보 존재 여부 체크 */
	void recvCheckUserExist();

	/* 가입 & 탈퇴*/
	void recvRegister();
	void recvWithDraw();
	
};

#endif