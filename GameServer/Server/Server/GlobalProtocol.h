#ifndef __GLOBALPROTOCOL_H__
#define __GLOBALPROTOCOL_H__

enum LICENCE_PROTOCOL {
	CLIENT_PASSWORD,				// Ŭ�󿡼� ��й�ȣ�� ����[ 16����Ʈ ]
	SERVER_RESULT					// ��й�ȣ �˻� ��� [ bool ]
};
enum GAME_PROTOCOL
{
	SERVER_GAMESTART = 100,		    // ���� ���� �˸� [  ]

	SERVER_POSITION_ENEMY,          // �� ĳ�� ��ġ [ ĳ�� ��(int), ���⺤��(x, y), ĳ����(x, y) X 4 ]
	SERVER_POSITION_PLAYER,         // ������ �� ĳ�� ��ǥ [ ĳ�� ��(int), ���⺤��(x, y), ĳ����(x, y) X 4 ]

	CLIENT_JOYSTICK,                // joystick ����  [ float ]
	SERVER_JOYSTICK,                // joystick ����  [ bool, float ]

	CLIENT_COLLISION_WALL,          // �� �浹 [  ]
	SERVER_COLLISION_WALL,          // �� �浹 �ǵ�� [ bool, ���� ����(x, y), ��ġ(x, y) ]

	CLIENT_COLLISION_OBSTACLE,      // ��ֹ� �浹 [ ��ֹ� ����(int) ]
	SERVER_COLLISION_OBSTACLE,      // ��ֹ� �浹 �ǵ�� [ bool, ���� ����(x, y), ��ġ(x, y) ]

	CLIENT_COLLISION_GIMMICK,       // ��� �浹 [ ��� idx(int) ]
	CLIENT_COLLISION_CAT,	       // ����� �浹 [ ���� idx(int) ]
	SERVER_COLLISION_GIMMICK,       // ��� �浹 �ǵ�� [ bool, ��� idx(int), �ʵ� idx, int step, ... ]
									/*
										���idx			step			packing
										6(������)		0(�浹)			���� ����(x, y), ��ġ(x, y)
										6(������)		1(����)			crtCount, (idx, hp) X N
										6(������)		2(������)		-
										7(��)			0				speed
										8,9(��)			0				���� ����(x, y), ��ġ(x, y)
										10(����)
										11(����)			0(����ȹ��)		int score
										11(����)			1(������ü������)
										12(����)			0(�̵��浹)		���� ����, ��ġ, int ��������
										12(����)			1(�����浹)		���� ����, ��ġ, int ��������, ȸ�� �����(ü���� ��� crtCount, (idx, hp) X N)
										12(����)			2(������)		
										13(�����)		0(�浹)			ģ��� ī��Ʈ, ���� ����(x, y), ��ġ(x, y)
										13(�����)		1(Ȱ��)			
										13(�����)		2(����)			���� ����(x, y), ��ġ(x, y)
										13(�����)		3(ȹ��)			����idx, �ڿ�������
										13(�����)		4(��Ȱ��)		
									*/

	CLIENT_ACTIVE_MOVE,             // �̶� �ֶO [ ]
	SERVER_ACTIVE_MOVE,             // �̵� ���� [ bool, float ��ü�ӵ� ]
	CLIENT_ACTIVE_ATTACK,           // ���� ���� [ ]
	SERVER_ACTIVE_ATTACK,           // ���� ���� [ bool, float ��ü�ӵ� ]
	CLIENT_ACTIVE_GUARD,            // ��� ���� [ ]
	SERVER_ACTIVE_GUARD,            // ��� ���� [ bool, float ��ü�ӵ� ]

	CLIENT_ATTACK_HIT,              // ���� ���� - �浹 [ index ]
	SERVER_ATTACK_HIT,              // ���� ���� - �浹 [ bool, ���� ����(x, y), ��ġ(x, y), index, int hp ]

	SERVER_RESPAWN_GIMMICK,         // ��� ����� [ ��� ����(int) ]
	SERVER_TIME,                    // Ÿ�̸� �ǵ�� [ int ]

	CLIENT_TEMPLE_ENTER,			// [bool enter]
	SERVER_TEMPLE_ENTER,			// [bool chance, gague]
	CLIENT_TEMPLE_CASTING,			// [bool start]
	SERVER_TEMPLE_CASTING,			// [bool player, bool start, float speed, float gauge, bool normaly]
	SERVER_TEMPLE_SUCCESS,			// [bool player]

	SERVER_PING,
	CLIENT_PONG,

	CLIENT_SKILL,					// [slot_idx, step, ...]
	SERVER_SKILL,					// [bool, slot_idx, step, ...]

	CLIENT_ITEMSKILL,				// [slot_idx, step, ...]
	SERVER_ITEMSKILL,				// [bool, slot_idx, step, ...]

	CLIENT_GAMEOBJECT,				// [obj_idx, step, ...]
	SERVER_GAMEOBJECT,				// [obj_idx, step, ...]

	SERVER_DATA_PLAYER,				// [ int bitmask, ... ]

	CLIENT_GOOGLE_JOIN_CHECK,		// [ string id ]
	SERVER_GOOGLE_JOIN_CHECK,		// [ bool success ]
	CLIENT_GOOGLE_JOIN,				// [ string id, string nick ]
	SERVER_GOOGLE_JOIN,				// [ bool success ]
	CLIENT_GOOGLE_LOGIN,			// [ string id ]
	SERVER_GOOGLE_LOGIN,			// [ bool success ]

	SERVER_USERDATA,				// [ ���� ������ ]
	CLIENT_SET_SUBCHARACTER,		// [ int ����idx, int sub_idx ]
	SERVER_SET_SUBCHARACTER,		// [ success ]
	CLIENT_SET_ITEMSKILL,			// [ int ����idx, int item_idx ]
	SERVER_SET_ITEMSKILL,			// [ success ]
	CLIENT_SET_COSTUME,				// [ int ����idx, int costume_idx ]
	SERVER_SET_COSTUME,				// [ success ]

	CLIENT_REQ_MATCHING,			// [ ]
	SERVER_REQ_MATCHING,			// [ bool isPlayer1, e_nick, e_leaderCostumeIdx, e_subIdx1, e_subIdx2, e_subIdx3, e_itemIdx1, e_itemIdx2, e_itemIdx3 ]
	
	SERVER_GAMEFINISH,				// [ int result, lv, extraExp, totalExp, extraGold, totalGold, extraRankPoint, totalRankPoint ]
	CLIENT_READY,					// [ ]
	CLIENT_CANCEL_MATCHING,			// [ ]
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

	CLIENT_ENTER_MATCHING2,			// [��Ī ��ȣ]
	SERVER_ENTER_MATCHING2,			// [��Ī ��ȣ, float field x1, y1, x2, y2, float crt������, float obstacle ������,
									// (stat, slide, speed, ĳ�� ��, (index, pos, maxHp, hp) X 4) X 2
									// ]
	SERVER_MATCHING_FRAME,			// [��Ī ��ȣ, float gauge, uint frame, (direction, slide, ĳ�� ��, pos X ĳ�� ��) X 2 ]
	CLIENT_LEAVE_MATCHING,			// [��Ī ��ȣ]
	SERVER_INFO_OBSTACLE,			// [��Ī ��ȣ, ��ֹ� ��, (��ǥ) X ��ֹ� ��]
	SERVER_CHANGE_STATE,			// [��Ī ��ȣ, int player_index, int stat, float speed]
	SERVER_EVENT_COLLISION_WALL,	// [ ��ȣ, int player_index, char* ����]
	SERVER_HIT_ATTACK,				// ��Ī�⺻���� [��Ī ��ȣ, bool �浹, bool ����, int �� index, �� ĳ�� ��, (index, �� hp) X ĳ�� ��]
	SERVER_EVENT_COLLISION_OBSTACLE,// [ ��ȣ, int player_index, char* ����]
	SERVER_DEBUG_PING,				// [ ��Ī ��ȣ, int player_index, int ping]
	
	SERVER_INFO_GIMMICK,			// [��Ī ��ȣ, ��� ��, (��� type, ����ü) X ��� ��]
	SERVER_CHANGE_TEMPLE,			// [��Ī ��ȣ, Temple  ����ü]
	SERVER_EVENT_TEMPLE,			// sendEvent�� ����
};

enum DB_PROTOCOL
{
	/* ���̺� ���� */
	GAMESERVER_REQ_SET_BASICINFO, // [ string ID, string nickname, int LV, int EXP, int gold, int gem, int gemOfVictory, int costume ]
	GAMESERVER_REQ_SET_MATCHINFO, // [ string ID, int matchingCount, int victoryCount, int victoryStreak, int loseStreak, int score ]
	GAMESERVER_REQ_SET_CHARACTER_DACK, // [ string ID, int leaderCharacter, int subCharacter1, int subCharacter2, int subCharacter3 ]
	GAMESERVER_REQ_SET_ITEM_DACK, // [ string ID, int itemSlot1, int itemSlot2, int itemSlot3 ]
	GAMESERVER_REQ_SET_EMOTION_DACK, // [ string ID, int emotionSlot1, int emotionSlot2, int emotionSlot3 ]
	GAMESERVER_REQ_SET_CHARACTER_INVENTORY, // [ string ID, int recordCnt, costumeIdx X recordCnt ]
	GAMESERVER_REQ_SET_ITEM_INVENTORY, // [ string ID, int recordCnt, vector<(int idx)> ]
	GAMESERVER_REQ_SET_COSTUME_INVENTORY, // [ string ID, int recordCnt, vector<(int idx)> ]
	GAMESERVER_REQ_SET_EMOTION_INVENTORY, // [ string ID, int recordCnt, vector<(int idx)> ]

	DBSERVER_RES_SET_BASICINFO, // [ string ID, bool isSuccess ]
	DBSERVER_RES_SET_MATCHINFO, // [ string ID, bool isSuccess ]
	DBSERVER_RES_SET_CHARACTER_DACK, // [ string ID, bool isSuccess ]
	DBSERVER_RES_SET_ITEM_DACK, // [ string ID, bool isSuccess ]
	DBSERVER_RES_SET_EMOTION_DACK, // [ string ID, bool isSuccess ]
	DBSERVER_RES_SET_CHARACTER_INVENTORY, // [ string ID, bool isSuccess ]
	DBSERVER_RES_SET_ITEM_INVENTORY, // [ string ID, bool isSuccess ]
	DBSERVER_RES_SET_COSTUME_INVENTORY, // [ string ID, bool isSuccess ]
	DBSERVER_RES_SET_EMOTION_INVENTORY, // [ string ID, bool isSuccess ]

	/* ���̺� Ž�� */
	GAMESERVER_REQ_BASICINFO, // [ string ID ]
	GAMESERVER_REQ_MATCHINFO, // [ string ID ]
	GAMESERVER_REQ_CHARACTER_DACK, // [ string ID ]
	GAMESERVER_REQ_ITEM_DACK, // [ string ID ]
	GAMESERVER_REQ_EMOTION_DACK, // [ string ID ]
	GAMESERVER_REQ_COSTUME_INVENTORY, // [ string ID ]
	GAMESERVER_REQ_ITEM_INVENTORY, // [ string ID ]
	GAMESERVER_REQ_EMOTION_INVENTORY, // [ string ID ]
	GAMESERVER_REQ_CHARACTER_INVENTORY, // [ string ID ]

	DBSERVER_RES_BASICINFO, // [ string ID, string nickname, int LV, int EXP, int gold, int gem, int gemOfVictory, int costume ]
	DBSERVER_RES_MATCHINFO, // [ string ID, int matchingCount, int victoryCount, int victoryStreak, int loseStreak, int score ]
	DBSERVER_RES_CHARACTER_DACK, // [ string ID, int leaderCharacter, int subCharacter1, int subCharacter2, int subCharacter3 ]
	DBSERVER_RES_ITEM_DACK, // [ string ID, int itemSlot1, int itemSlot2, int itemSlot3 ]
	DBSERVER_RES_EMOTION_DACK, // [ string ID, int emotionSlot1, int emotionSlot2, int emotionSlot3 ]
	DBSERVER_RES_CHARACTER_INVENTORY, // [ string ID, int recordCnt, costumeIdx X recordCnt ]
	DBSERVER_RES_ITEM_INVENTORY, // [ string ID, int recordCnt, vector<(int idx)> ]
	DBSERVER_RES_COSTUME_INVENTORY, // [ string ID, int recordCnt, vector<(int idx)> ] 
	DBSERVER_RES_EMOTION_INVENTORY, // [ string ID, int recordCnt, vector<(int idx)> ]

	/* ���� ���� Ȯ�� */
	GAMESERVER_REQ_CHECK_USEREXIST, // [ string ID ]

	DBSERVER_RES_CHECK_USEREXIST, // [ string ID, bool isExist ]

	/* ���� ���� */
	// REGISTER(����)
	GAMESERVER_REQ_REGISTER = 9000, // [ string ID, string nickname ]
	DBSERVER_RES_REGISTER, // [ string ID, bool isSuccess ]
	// Withdraw(Ż��)
	GAMESERVER_REQ_WITHDRAW, // [ string ID ]
	DBSERVER_RES_WITHDRAW, // [ string ID, bool isSuccess ]

	GAMESERVER_MATCHEND = 12345, //[string:user1ID, string:user2ID, bool:isUser1Win]
};

#endif
