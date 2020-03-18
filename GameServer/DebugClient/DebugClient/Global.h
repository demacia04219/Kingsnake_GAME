#ifndef GLOBAL_H
#define GLOBAL_H

/*#pragma warning(disable:4996)
#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <stdio.h>*/


#define ENCRYPT_KEY 549815485
//#define SERVER_IP   "58.141.122.30"	// �б�
//#define SERVER_IP   "121.174.95.151"	// G-Star
//#define SERVER_IP   "58.142.29.110"	// ���� ��
#define SERVER_IP   "127.0.0.1"
#define SERVER_PORT 1271

#define PASSWORD_DEBUG	{-2001832157,  218035469,  -597923185, 1725978516}

enum LOG_TYPE {
	NORMAL,			// �⺻
	COLLISION_WALL_OBSTACLE,	// ��, ��ֹ� �浹 Ÿ��
	TEMPLE,			// ���� ����
};
enum PLAYER_STATE {
	MOVE = 0, ATTACK, GUARD, DEATH
};
enum USER_STATE {
	LICENCE,		// Ŭ�� ����
	CHECK,			// Ŭ�� ����, ������ ���� üũ, �α���, �ߺ� ���� üũ
	MAIN,			// �κ�, ���� ��
	MATCHING,		// 2�� ��Ī
	DEBUG_MAIN,		// ����� ����
};

enum LICENCE_PROTOCOL {
	CLIENT_PASSWORD,				// Ŭ�󿡼� ��й�ȣ�� ����[ 16����Ʈ ]
	SERVER_RESULT					// ��й�ȣ �˻� ��� [ bool ]
};

enum DEBUG_PROTOCOL 
{
	CLIENT_USERLIST = 1000,			// []
	SERVER_USERLIST,				// [�� ���� ��ȣ, ������, (ip, serial, stat) X ������]
	SERVER_CONNECT_USER,			// [ip, serial, stat]
	SERVER_DISCONNECT_USER,			// [serial]

	CLIENT_MATCHINGLIST,			// []
	SERVER_MATCHINGLIST,			// [��Ī ��, (��Ī ��ȣ, 1p serial, 2p serial) X ��Ī ��]
	SERVER_CREATE_MATCHING,			// [��Ī ��ȣ, 1p serial, 2p serial]
	SERVER_DESTROY_MATCHING,		// [��Ī ��ȣ]

	CLIENT_ENTER_MATCHING,			// [��Ī ��ȣ]
	SERVER_ENTER_MATCHING,			// [��Ī ��ȣ, float field x1, y1, x2, y2, float crt������, float obstacle ������,
									// (stat, joystick, speed, ĳ�� ��, (index, pos, maxHp, hp) X ĳ�� ��) X 2
									// ]
	SERVER_MATCHING_FRAME,			// [��Ī ��ȣ, float gauge, uint frame, (direction, slide, ĳ�� ��, pos X ĳ�� ��) X 2 ]
	CLIENT_LEAVE_MATCHING,			// [��Ī ��ȣ]
	SERVER_INFO_OBSTACLE,			// [��Ī ��ȣ, ��ֹ� ��, (��ǥ) X ��ֹ� ��]
	SERVER_CHANGE_STATE,			// [��Ī ��ȣ, int player_index, int stat, float speed]
	SERVER_COLLISION_WALL,			// [ ��ȣ, int player_index, char* ����]
	SERVER_HIT_ATTACK,				// ��Ī�⺻���� [��Ī ��ȣ, bool �浹, bool ����, int �� index, �� ĳ�� ��, (index, �� hp) X ĳ�� ��]
	SERVER_COLLISION_OBSTACLE,		// [ ��ȣ, int player_index, char* ����]
	SERVER_DEBUG_PING,				// [ ��Ī ��ȣ, int player_index, int ping]
	
	SERVER_INFO_GIMMICK,			// [��Ī ��ȣ, ��� ��, (��� type, ����ü) X ��� ��]
	SERVER_CHANGE_TEMPLE,			// [��Ī ��ȣ, Temple  ����ü]
	SERVER_EVENT_TEMPLE,			// sendEvent�� ����
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
