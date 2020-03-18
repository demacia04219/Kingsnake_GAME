#ifndef	__USER_H__
#define	__USER_H__

#include "IOCPClient.h"
#include "Global.h"
#include "UserData.h"
#include "Packet.h"

class Matching;
struct LoginLicence;
class State;


// ���� Ŭ�� or ����� Ŭ�� ���� �ʱ� ������Ʈ
#define GAME_INTRO_STATE USER_STATE::CHECK
#define DEBUG_INTRO_STATE USER_STATE::DEBUG

class User : public IOCPClient
{
private:
	CRITICAL_SECTION cs;
	State* currentState;
	UserData data;		// ���� ����
	bool isLogin;
	int serial;

	~User();

public:
	User();

	// State
	int getSerial();
	void setState(USER_STATE state);
	USER_STATE getStateType();
	State* getStateObject();
	bool isStateOf(USER_STATE state);

	// Others..
	bool setMatching(Matching* matching);
	Matching* getMatching();
	LoginLicence* getLoginLicence();
	UserData& getUserData();
	bool isDebugClient();
	bool isGameClient();
	bool getIsLogin();
	void setIsLogin(bool);

	// DB Server
	void recvUserData(UserData::USER_DATA);

	// IOCP
	void recvProcess()					override;
	void acceptProcess()				override;
	void disconnectProcess()			override;
	void errorProcess(IOCP_ERROR err)	override;

	void EnterCS();
	void LeaveCS();

	// ping
private:
	long ping;// ��
	short pong;
	short _avg_ping;// 10�� ���
	short _avg_ping_sub, _avg_ping_count;// 10�� ��� ��� ����
public:
	short getPing();
	short getAvgPing();
	void setPong();
	void setPing();
};

#endif