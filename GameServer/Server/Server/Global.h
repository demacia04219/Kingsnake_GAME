#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#pragma warning (disable:4267)
#pragma warning (disable:4099)
#pragma warning (disable:4244)

#include "Vector.h"

enum USER_STATE {
	LICENCE,		// Ŭ�� ����
	CHECK,			// Ŭ�� ����, ������ ���� üũ, �α���, �ߺ� ���� üũ
	LOBBY,			// �κ�, ���� ��
	MATCHING,		// 2�� ��Ī
	DEBUG,			// ����� Ŭ��
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
	GIMMICK_MARSH,		// ��
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
	CURSE,			// ����
	TELEPORT,
	ALLUREMENT,		// ��Ȥ
};
enum FIELDOBJECT_TYPE
{
	NONE,
	SWORD=1,		// �׸��� �� ����ü
	BUBBLE,			// ��� ���� ����ü
	RABBIT_BOMB,	// �����䳢 ����ü
	ARROW,			// ȭ�� ���� ����ü
	BLACK_HOLE,		// ����Ʈ �ҷο� ����ü
	MAGNET_ARROW,	// ��Ȥ ����ü
};
enum MATCHING_TYPE// �⺻ ��Ī ������ �� ���� Ÿ��
{
	BASIC = 1
};
enum PACKING_PLAYER// �÷��̾� ���� ��ŷ ��Ʈ����ũ
{
	PP_NULL					= 0,
	PP_RESPAWN				= (1 << 0),		// ��Ȱ					[bool, direction, position]
	PP_POSITION				= (1 << 1),		// ĳ�� ��ġ				[bool, direction, count, (idx, position) * count]
	PP_MAXHP_ALL			= (1 << 2),		// �ִ�ü��				[bool, count, (idx, maxHP) * count]
	PP_HP_ALL				= (1 << 3),		// ü��					[bool, count, (idx, hp) * count]
	PP_SPEED				= (1 << 4),		// ���ǵ�				[bool, speed]
	PP_SKILLGAUGE			= (1 << 5),		// ��ų ������
	PP_MAXSTAMINA			= (1 << 6),		// �ִ� ���׹̳�
	PP_STAMINA				= (1 << 7),		// ���׹̳�
	PP_MAXSKILLGAUGE		= (1 << 8),		// �ִ� ��ų ������
	PP_DECREASE_ATTACK		= (1 << 9),		// ���� ���׹̳� ���ҷ�(�ʴ�)
	PP_DECREASE_GUARD		= (1 << 10),	// ��� ���׹̳� ���ҷ�(�ʴ�)
	PP_DECREASE_CASTING		= (1 << 11),	// ĳ���� ���׹̳� ���ҷ�(�ʴ�)
	PP_RECOVERY_STAMINA		= (1 << 12),	// ���׹̳� ȸ����(�ʴ�)
	PP_RECOVERY_SKILLGAUGE	= (1 << 13),	// ��ų ������ ȸ����(�ʴ�)
	PP_SCORE				= (1 << 14),	// ����					[bool, score]
	PP_COOLTIME_SKILL		= (1 << 15),	// ��ų ��Ÿ��(��)		[colltime * 3]
	PP_COOLTIME_ITEM		= (1 << 16),	// ������ ��Ÿ��(��)		[cooltime * 3]
	PP_RECOVERY_HP			= (1 << 17),	// ü�� ȸ����(�ʴ�)		[bool, recoveryPower]
	PP_BLEEDING				= (1 << 18),	// ����					[bool, count, (idx, bool start, float damage(�ʴ�)) * count]
	PP_STUN					= (1 << 19),	// ����(�̼� 0, ����X)	[bool, bool start, speed]
	PP_CURSE				= (1 << 20),	// ����(���̽�ƽ �ݴ�)	[bool start]
	PP_IGNORE_CONTROL		= (1 << 21),	// ����X					[bool start]

	// ������ ������ ��� �� ����
	PP_ALL_TO_ME			= 0xFFFFFFFF,
	// ��뿡�� ���� �� �ִ� ��� �� ����
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

// DB ���� ����
	#define DBSERVERPORT	1721
	//#define	DBSERVERIP		"58.141.122.30"
	#define	DBSERVERIP			"35.243.110.64" // ??sssss

// �� ���� ����
	#define SERVER_VERSION	"20181219_1250"
	#define SERVERPORT		1271
	#define PASSWORD_GAME	{  226812357, 1282537491, -1098234567, -615825637}
	#define PASSWORD_DEBUG	{-2001832157,  218035469,  -597923185, 1725978516}

// �����
	#define DEBUG_MATCHING	false
	
#endif