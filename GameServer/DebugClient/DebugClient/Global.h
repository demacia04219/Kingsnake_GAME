#ifndef GLOBAL_H
#define GLOBAL_H

/*#pragma warning(disable:4996)
#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <stdio.h>*/


#define ENCRYPT_KEY 549815485
//#define SERVER_IP   "58.141.122.30"	// 학교
//#define SERVER_IP   "121.174.95.151"	// G-Star
//#define SERVER_IP   "58.142.29.110"	// 서울 집
#define SERVER_IP   "127.0.0.1"
#define SERVER_PORT 1271

#define PASSWORD_DEBUG	{-2001832157,  218035469,  -597923185, 1725978516}

enum LOG_TYPE {
	NORMAL,			// 기본
	COLLISION_WALL_OBSTACLE,	// 벽, 장애물 충돌 타입
	TEMPLE,			// 템플 관련
};
enum PLAYER_STATE {
	MOVE = 0, ATTACK, GUARD, DEATH
};
enum USER_STATE {
	LICENCE,		// 클라 인증
	CHECK,			// 클라 버전, 데이터 버전 체크, 로그인, 중복 접속 체크
	MAIN,			// 로비, 상점 등
	MATCHING,		// 2인 매칭
	DEBUG_MAIN,		// 디버그 메인
};

enum LICENCE_PROTOCOL {
	CLIENT_PASSWORD,				// 클라에서 비밀번호를 받음[ 16바이트 ]
	SERVER_RESULT					// 비밀번호 검사 결과 [ bool ]
};

enum DEBUG_PROTOCOL 
{
	CLIENT_USERLIST = 1000,			// []
	SERVER_USERLIST,				// [내 유저 번호, 유저수, (ip, serial, stat) X 유저수]
	SERVER_CONNECT_USER,			// [ip, serial, stat]
	SERVER_DISCONNECT_USER,			// [serial]

	CLIENT_MATCHINGLIST,			// []
	SERVER_MATCHINGLIST,			// [매칭 수, (매칭 번호, 1p serial, 2p serial) X 매칭 수]
	SERVER_CREATE_MATCHING,			// [매칭 번호, 1p serial, 2p serial]
	SERVER_DESTROY_MATCHING,		// [매칭 번호]

	CLIENT_ENTER_MATCHING,			// [매칭 번호]
	SERVER_ENTER_MATCHING,			// [매칭 번호, float field x1, y1, x2, y2, float crt반지름, float obstacle 반지름,
									// (stat, joystick, speed, 캐릭 수, (index, pos, maxHp, hp) X 캐릭 수) X 2
									// ]
	SERVER_MATCHING_FRAME,			// [매칭 번호, float gauge, uint frame, (direction, slide, 캐릭 수, pos X 캐릭 수) X 2 ]
	CLIENT_LEAVE_MATCHING,			// [매칭 번호]
	SERVER_INFO_OBSTACLE,			// [매칭 번호, 장애물 수, (좌표) X 장애물 수]
	SERVER_CHANGE_STATE,			// [매칭 번호, int player_index, int stat, float speed]
	SERVER_COLLISION_WALL,			// [ 번호, int player_index, char* 유형]
	SERVER_HIT_ATTACK,				// 매칭기본공격 [매칭 번호, bool 충돌, bool 공격, int 내 index, 적 캐릭 수, (index, 적 hp) X 캐릭 수]
	SERVER_COLLISION_OBSTACLE,		// [ 번호, int player_index, char* 유형]
	SERVER_DEBUG_PING,				// [ 매칭 번호, int player_index, int ping]
	
	SERVER_INFO_GIMMICK,			// [매칭 번호, 기믹 수, (기믹 type, 구조체) X 기믹 수]
	SERVER_CHANGE_TEMPLE,			// [매칭 번호, Temple  구조체]
	SERVER_EVENT_TEMPLE,			// sendEvent로 실행
};

enum MESSAGE
{
	USERLIST = 1000,
	CONNECT,
	DISCONNECT,
	MATCHINGLIST,
	CREATE_MATCHING,
	DESTROY_MATCHING
};



#endif
