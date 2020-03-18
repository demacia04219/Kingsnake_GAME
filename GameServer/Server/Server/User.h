#ifndef	__USER_H__
#define	__USER_H__

#include "IOCPClient.h"
#include "Global.h"
#include "UserData.h"
#include "Packet.h"

class Matching;
struct LoginLicence;
class State;


// 게임 클라 or 디버그 클라 접속 초기 스테이트
#define GAME_INTRO_STATE USER_STATE::CHECK
#define DEBUG_INTRO_STATE USER_STATE::DEBUG

class User : public IOCPClient
{
private:
	CRITICAL_SECTION cs;
	State* currentState;
	UserData data;		// 계정 정보
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
	long ping;// 핑
	short pong;
	short _avg_ping;// 10초 평균
	short _avg_ping_sub, _avg_ping_count;// 10초 평균 계산 과정
public:
	short getPing();
	short getAvgPing();
	void setPong();
	void setPing();
};

#endif