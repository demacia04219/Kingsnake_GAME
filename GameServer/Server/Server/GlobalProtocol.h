#ifndef __GLOBALPROTOCOL_H__
#define __GLOBALPROTOCOL_H__

enum LICENCE_PROTOCOL {
	CLIENT_PASSWORD,				// 클라에서 비밀번호를 받음[ 16바이트 ]
	SERVER_RESULT					// 비밀번호 검사 결과 [ bool ]
};
enum GAME_PROTOCOL
{
	SERVER_GAMESTART = 100,		    // 게임 시작 알림 [  ]

	SERVER_POSITION_ENEMY,          // 적 캐릭 위치 [ 캐릭 수(int), 방향벡터(x, y), 캐릭터(x, y) X 4 ]
	SERVER_POSITION_PLAYER,         // 보정된 내 캐릭 좌표 [ 캐릭 수(int), 방향벡터(x, y), 캐릭터(x, y) X 4 ]

	CLIENT_JOYSTICK,                // joystick 각도  [ float ]
	SERVER_JOYSTICK,                // joystick 각도  [ bool, float ]

	CLIENT_COLLISION_WALL,          // 벽 충돌 [  ]
	SERVER_COLLISION_WALL,          // 벽 충돌 피드백 [ bool, 방향 벡터(x, y), 위치(x, y) ]

	CLIENT_COLLISION_OBSTACLE,      // 장애물 충돌 [ 장애물 종류(int) ]
	SERVER_COLLISION_OBSTACLE,      // 장애물 충돌 피드백 [ bool, 방향 벡터(x, y), 위치(x, y) ]

	CLIENT_COLLISION_GIMMICK,       // 기믹 충돌 [ 기믹 idx(int) ]
	CLIENT_COLLISION_CAT,	       // 몽블랑 충돌 [ 보석 idx(int) ]
	SERVER_COLLISION_GIMMICK,       // 기믹 충돌 피드백 [ bool, 기믹 idx(int), 필드 idx, int step, ... ]
									/*
										기믹idx			step			packing
										6(만도라)		0(충돌)			방향 벡터(x, y), 위치(x, y)
										6(만도라)		1(폭발)			crtCount, (idx, hp) X N
										6(만도라)		2(리스폰)		-
										7(늪)			0				speed
										8,9(성)			0				방향 벡터(x, y), 위치(x, y)
										10(제단)
										11(보석)			0(보석획득)		int score
										11(보석)			1(보석전체리스폰)
										12(나무)			0(이동충돌)		방향 벡터, 위치, int 나무상태
										12(나무)			1(공격충돌)		방향 벡터, 위치, int 나무상태, 회복 결과값(체력의 경우 crtCount, (idx, hp) X N)
										12(나무)			2(리스폰)		
										13(몽블랑)		0(충돌)			친사람 카운트, 방향 벡터(x, y), 위치(x, y)
										13(몽블랑)		1(활성)			
										13(몽블랑)		2(보정)			방향 벡터(x, y), 위치(x, y)
										13(몽블랑)		3(획득)			보석idx, 자원게이지
										13(몽블랑)		4(비활성)		
									*/

	CLIENT_ACTIVE_MOVE,             // 이똥 쌍턔 [ ]
	SERVER_ACTIVE_MOVE,             // 이동 상태 [ bool, float 전체속도 ]
	CLIENT_ACTIVE_ATTACK,           // 공격 상태 [ ]
	SERVER_ACTIVE_ATTACK,           // 공격 상태 [ bool, float 전체속도 ]
	CLIENT_ACTIVE_GUARD,            // 방어 상태 [ ]
	SERVER_ACTIVE_GUARD,            // 방어 상태 [ bool, float 전체속도 ]

	CLIENT_ATTACK_HIT,              // 공격 상태 - 충돌 [ index ]
	SERVER_ATTACK_HIT,              // 공격 상태 - 충돌 [ bool, 방향 벡터(x, y), 위치(x, y), index, int hp ]

	SERVER_RESPAWN_GIMMICK,         // 기믹 재생성 [ 기믹 종류(int) ]
	SERVER_TIME,                    // 타이머 피드백 [ int ]

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

	SERVER_USERDATA,				// [ 유저 데이터 ]
	CLIENT_SET_SUBCHARACTER,		// [ int 슬롯idx, int sub_idx ]
	SERVER_SET_SUBCHARACTER,		// [ success ]
	CLIENT_SET_ITEMSKILL,			// [ int 슬롯idx, int item_idx ]
	SERVER_SET_ITEMSKILL,			// [ success ]
	CLIENT_SET_COSTUME,				// [ int 슬롯idx, int costume_idx ]
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
	SERVER_USERLIST,				// [내 유저 번호, 유저수, (ip, serial, stat) X 유저수]
	SERVER_CONNECT_USER,			// [ip, serial, stat]
	SERVER_DISCONNECT_USER,			// [serial]
	
	CLIENT_MATCHINGLIST,			// []
	SERVER_MATCHINGLIST,			// [매칭 수, (매칭 번호, 1p serial, 2p serial) X 매칭 수]
	SERVER_CREATE_MATCHING,			// [매칭 번호, 1p serial, 2p serial]
	SERVER_DESTROY_MATCHING,		// [매칭 번호]

	CLIENT_ENTER_MATCHING2,			// [매칭 번호]
	SERVER_ENTER_MATCHING2,			// [매칭 번호, float field x1, y1, x2, y2, float crt반지름, float obstacle 반지름,
									// (stat, slide, speed, 캐릭 수, (index, pos, maxHp, hp) X 4) X 2
									// ]
	SERVER_MATCHING_FRAME,			// [매칭 번호, float gauge, uint frame, (direction, slide, 캐릭 수, pos X 캐릭 수) X 2 ]
	CLIENT_LEAVE_MATCHING,			// [매칭 번호]
	SERVER_INFO_OBSTACLE,			// [매칭 번호, 장애물 수, (좌표) X 장애물 수]
	SERVER_CHANGE_STATE,			// [매칭 번호, int player_index, int stat, float speed]
	SERVER_EVENT_COLLISION_WALL,	// [ 번호, int player_index, char* 유형]
	SERVER_HIT_ATTACK,				// 매칭기본공격 [매칭 번호, bool 충돌, bool 공격, int 내 index, 적 캐릭 수, (index, 적 hp) X 캐릭 수]
	SERVER_EVENT_COLLISION_OBSTACLE,// [ 번호, int player_index, char* 유형]
	SERVER_DEBUG_PING,				// [ 매칭 번호, int player_index, int ping]
	
	SERVER_INFO_GIMMICK,			// [매칭 번호, 기믹 수, (기믹 type, 구조체) X 기믹 수]
	SERVER_CHANGE_TEMPLE,			// [매칭 번호, Temple  구조체]
	SERVER_EVENT_TEMPLE,			// sendEvent로 실행
};

enum DB_PROTOCOL
{
	/* 테이블 변경 */
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

	/* 테이블 탐색 */
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

	/* 계정 유무 확인 */
	GAMESERVER_REQ_CHECK_USEREXIST, // [ string ID ]

	DBSERVER_RES_CHECK_USEREXIST, // [ string ID, bool isExist ]

	/* 계정 변경 */
	// REGISTER(가입)
	GAMESERVER_REQ_REGISTER = 9000, // [ string ID, string nickname ]
	DBSERVER_RES_REGISTER, // [ string ID, bool isSuccess ]
	// Withdraw(탈퇴)
	GAMESERVER_REQ_WITHDRAW, // [ string ID ]
	DBSERVER_RES_WITHDRAW, // [ string ID, bool isSuccess ]

	GAMESERVER_MATCHEND = 12345, //[string:user1ID, string:user2ID, bool:isUser1Win]
};

#endif
