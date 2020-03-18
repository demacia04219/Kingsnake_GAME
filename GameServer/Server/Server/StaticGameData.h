#ifndef __GAME_DATA_H__
#define __GAME_DATA_H__

#include "Global.h"
#include <vector>
#include <map>
#include <string>

// .csv ���� ����
	#define CSV_GIMMICK					"csv/gimmick.csv"
	#define CSV_SKILL					"csv/skill.csv"
	#define CSV_ITEM					"csv/itemSkill.csv"
	#define CSV_LEADER					"csv/leaderCharacter.csv"
	#define CSV_SUB						"csv/subCharacter.csv"
	#define CSV_PASSIVE					"csv/passiveSkill.csv"
	#define CSV_CRT_EFFECT				"csv/characterEffect.csv"
	#define CSV_COSTUME					"csv/costume.csv"


	#define CSV_MATCHING1				"csv/matching.csv"
	#define CSV_MAP1					"csv/map.csv"


struct ActiveData {	// ��Ƽ�� (��ų, �����ۿ� ����)
	int index;
	std::string name;
	float coolTime;		// ��Ÿ��(��)
	float runTime;		// �����ð�(��)
	float gaugeCost;
	float data[7];
};

struct SkillData : public ActiveData {

};
struct ItemData : public ActiveData {

};

struct LeaderCharacterData {
	float maxHP;				// ü��
	float maxStamina;			// ���׹̳�
	float maxSkillGauge;		// ��ų ������
	float speed;				// ���ǵ�
	float recoveryStamina;		// ���׹̳� ȸ��(��)
	float damage;				// ���ݷ�
	float decrementStaminaAttack;	// ���� ���׹̳� ���ҷ�(��)
	float decrementStaminaDefense;	// ��� ���׹̳� ���ҷ�(��)
	float decrementStaminaCasting;	// ĳ���� ���׹̳� ���ҷ�	(��)
	float respawnDelay;			// ��Ȱ�ð�(��)
	float increaseSpeed;		// ���� ĳ���� ���Ҵ� ���ǵ� ������
	float recoverySkillGauge;	// ��ų ������ ȸ��(��)
	float attackRecovery;		// ���� ���� �� ��ų ������ ȸ��
};
struct SubCharacterData {
	int index;
	std::string name;
	int type;					// ���� ����
	int skill;					// ��� ��ų(idx)
	float extraStamina;			// ���� �߰� ���׹̳�
	float extraSpeed;			// ���� �߰� ���ǵ�
	float maxHP;				// ü��
	float damage;				// ���ݷ�
};
struct GimmickData
{
	// DB ����
	MAP_OBJECT_TYPE index;
	float radius;				// ������
	float data[5];
};

/* PassiveData�� ��ȹ �� ������� ���� */
struct PassiveData {
	int index;
	int type;					// ��������
	int count;					// count�� �̻� ȿ�� ����
	float data[2];
};

/* CharacterEffectData�� ��ȹ �� ������� ���� */
struct CharacterEffectData {	// ĳ���� �������� �� �߰� ����
	int index;
	int type;					// ��������
	int count;					// count���� �� ȿ�� ����
	float data[2];
};

struct CostumeData {
	int index;
	int subCharacter;				// � ĳ���Ϳ� ���� �ڽ�Ƭ����
	int price;					// ��(����)
};

class Obstacle;
class Gimmick;
class Temple;
class TCPClient;

struct MatchingData
{
	MATCHING_TYPE type;
	bool valid;				// ��ȿ�� ��Ī ���������� (csv �ε� ����)

	int frame_rate;
	float crt_radius;
	float obstacle_radius;

	Rect map_size;
	int map_block_width;
	int map_block_height;
	int playtime;			// ���� �ð�(���� ���� ���� �ð�)
	int finish_score;		// ���� ���� ���� ����

	Vector2 p1_startPosition;
	Vector2 p2_startPosition;
	Vector2 p1_startDirection;
	Vector2 p2_startDirection;

	std::vector<Obstacle*> obstacle;
	std::vector<Gimmick*> gimmick;
	std::vector<Vector2> catWayPoint;

	MatchingData(MATCHING_TYPE type):type(type),valid(true) {
		init(); 
	}
	~MatchingData() { dispose(); }
	void init();
	void dispose();

	Vector2 blockToField(Vector2 blockPos);// ��� ��ǥ�� ��Ī �ʵ� ��ǥ��
};

struct StaticGameData
{
	LeaderCharacterData leader;
	std::map<int, SkillData> skill;
	std::map<int, ItemData> item;
	std::map<int, SubCharacterData> sub;
	std::map<int, GimmickData> gimmick;
	std::map<int, PassiveData> passive;
	std::map<int, CharacterEffectData> crtEffect;
	std::map<int, CostumeData> costume;
	std::map<MATCHING_TYPE, MatchingData*> matching;

	StaticGameData();
	void load();
};

#endif