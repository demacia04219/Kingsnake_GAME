#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#pragma warning (disable:4267)
#pragma warning (disable:4099)
#pragma warning (disable:4244)

#include "Vector.h"

enum USER_STATE {
	LICENCE,		// 클라 인증
	CHECK,			// 클라 버전, 데이터 버전 체크, 로그인, 중복 접속 체크
	LOBBY,			// 로비, 상점 등
	MATCHING,		// 2인 매칭
	DEBUG,			// 디버그 클라
};
enum PLAYER_STATE {
	MOVE = 0, 
	ATTACK, 
	GUARD, 
	DEATH
}; 
enum MAP_OBJECT_TYPE
{
	IDLE = 0,
	OBSTACLE_TREE_SMALL = 1,
	OBSTACLE_TREE_MEDIUM,
	OBSTACLE_TREE_LARGE,
	OBSTACLE_ROCK,

	GIMMICK_BUSH,
	GIMMICK_MANDRA,
	GIMMICK_MARSH,		// 늪
	GIMMICK_CASTLE_BLUE,
	GIMMICK_CASTLE_RED,
	GIMMICK_TEMPLE,
	GIMMICK_GEM,
	GIMMICK_BLESSINGTREE,
	GIMMICK_CAT,
	NOT_USE_1,
	NOT_USE_2,
};
enum SKILL_TYPE
{
	SHADOWSWORD = 0,
	WINDPUSH,
	BUBBLETRAP,
};
enum ITEM_TYPE
{
	RABBIT,
	ARROWSNIPE,
	SCRATCH,
	NIGHTHOLLOW,
	CURSE,			// 저주
	TELEPORT,
	ALLUREMENT,		// 매혹
};
enum FIELDOBJECT_TYPE
{
	NONE,
	SWORD=1,		// 그림자 검 투사체
	BUBBLE,			// 방울 감옥 투사체
	RABBIT_BOMB,	// 원폭토끼 투사체
	ARROW,			// 화살 저격 투사체
	BLACK_HOLE,		// 나이트 할로우 투사체
	MAGNET_ARROW,	// 매혹 투사체
};
enum MATCHING_TYPE// 기본 매칭 정보와 맵 정보 타입
{
	BASIC = 1
};
enum PACKING_PLAYER// 플레이어 정보 패킹 비트마스크
{
	PP_NULL					= 0,
	PP_RESPAWN				= (1 << 0),		// 부활					[bool, direction, position]
	PP_POSITION				= (1 << 1),		// 캐릭 위치				[bool, direction, count, (idx, position) * count]
	PP_MAXHP_ALL			= (1 << 2),		// 최대체력				[bool, count, (idx, maxHP) * count]
	PP_HP_ALL				= (1 << 3),		// 체력					[bool, count, (idx, hp) * count]
	PP_SPEED				= (1 << 4),		// 스피드				[bool, speed]
	PP_SKILLGAUGE			= (1 << 5),		// 스킬 게이지
	PP_MAXSTAMINA			= (1 << 6),		// 최대 스테미나
	PP_STAMINA				= (1 << 7),		// 스테미나
	PP_MAXSKILLGAUGE		= (1 << 8),		// 최대 스킬 게이지
	PP_DECREASE_ATTACK		= (1 << 9),		// 공격 스테미나 감소량(초당)
	PP_DECREASE_GUARD		= (1 << 10),	// 방어 스테미나 감소량(초당)
	PP_DECREASE_CASTING		= (1 << 11),	// 캐스팅 스테미나 감소량(초당)
	PP_RECOVERY_STAMINA		= (1 << 12),	// 스테미나 회복량(초당)
	PP_RECOVERY_SKILLGAUGE	= (1 << 13),	// 스킬 게이지 회복량(초당)
	PP_SCORE				= (1 << 14),	// 점수					[bool, score]
	PP_COOLTIME_SKILL		= (1 << 15),	// 스킬 쿨타임(초)		[colltime * 3]
	PP_COOLTIME_ITEM		= (1 << 16),	// 아이템 쿨타임(초)		[cooltime * 3]
	PP_RECOVERY_HP			= (1 << 17),	// 체력 회복량(초당)		[bool, recoveryPower]
	PP_BLEEDING				= (1 << 18),	// 출혈					[bool, count, (idx, bool start, float damage(초당)) * count]
	PP_STUN					= (1 << 19),	// 스턴(이속 0, 조작X)	[bool, bool start, speed]
	PP_CURSE				= (1 << 20),	// 저주(조이스틱 반대)	[bool start]
	PP_IGNORE_CONTROL		= (1 << 21),	// 조작X					[bool start]

	// 나에게 보내는 모든 내 정보
	PP_ALL_TO_ME			= 0xFFFFFFFF,
	// 상대에게 보낼 수 있는 모든 내 정보
	PP_ALL_TO_ENEMY			= PP_SPEED | PP_SCORE | PP_HP_ALL | PP_MAXHP_ALL | PP_POSITION | PP_RECOVERY_HP | PP_BLEEDING | PP_STUN | PP_RESPAWN,
};


struct Rect {
	Vector2 leftBottom;
	Vector2 rightTop;
	Rect(Vector2 leftBottom, Vector2 rightTop) {
		this->leftBottom = leftBottom;
		this->rightTop = rightTop;
	}
	Rect() {}
	float getLeft() { return leftBottom.x; }
	float getRight() { return rightTop.x; }
	float getBottom() { return leftBottom.y; }
	float getTop() { return rightTop.y; }
};

// DB 서버 정의
	#define DBSERVERPORT	1721
	//#define	DBSERVERIP		"58.141.122.30"
	#define	DBSERVERIP			"35.243.110.64" // ??sssss

// 내 서버 정의
	#define SERVER_VERSION	"20181219_1250"
	#define SERVERPORT		1271
	#define PASSWORD_GAME	{  226812357, 1282537491, -1098234567, -615825637}
	#define PASSWORD_DEBUG	{-2001832157,  218035469,  -597923185, 1725978516}

// 디버깅
	#define DEBUG_MATCHING	false
	
#endif