#ifndef __GAME_DATA_H__
#define __GAME_DATA_H__

#include "Global.h"
#include <vector>
#include <map>
#include <string>

// .csv 파일 정보
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


struct ActiveData {	// 액티브 (스킬, 아이템에 쓰임)
	int index;
	std::string name;
	float coolTime;		// 쿨타임(초)
	float runTime;		// 시전시간(초)
	float gaugeCost;
	float data[7];
};

struct SkillData : public ActiveData {

};
struct ItemData : public ActiveData {

};

struct LeaderCharacterData {
	float maxHP;				// 체력
	float maxStamina;			// 스테미나
	float maxSkillGauge;		// 스킬 게이지
	float speed;				// 스피드
	float recoveryStamina;		// 스테미나 회복(초)
	float damage;				// 공격력
	float decrementStaminaAttack;	// 공격 스테미나 감소량(초)
	float decrementStaminaDefense;	// 방어 스테미나 감소량(초)
	float decrementStaminaCasting;	// 캐스팅 스테미나 감소량	(초)
	float respawnDelay;			// 부활시간(초)
	float increaseSpeed;		// 서브 캐릭터 감소당 스피드 증가량
	float recoverySkillGauge;	// 스킬 게이지 회복(초)
	float attackRecovery;		// 공격 성공 시 스킬 게이지 회복
};
struct SubCharacterData {
	int index;
	std::string name;
	int type;					// 전투 유형
	int skill;					// 사용 스킬(idx)
	float extraStamina;			// 리더 추가 스테미나
	float extraSpeed;			// 리더 추가 스피드
	float maxHP;				// 체력
	float damage;				// 공격력
};
struct GimmickData
{
	// DB 정보
	MAP_OBJECT_TYPE index;
	float radius;				// 반지름
	float data[5];
};

/* PassiveData는 기획 상 사용하지 않음 */
struct PassiveData {
	int index;
	int type;					// 전투유형
	int count;					// count명 이상 효과 적용
	float data[2];
};

/* CharacterEffectData는 기획 상 사용하지 않음 */
struct CharacterEffectData {	// 캐릭터 전투유형 별 추가 스텟
	int index;
	int type;					// 전투유형
	int count;					// count명일 때 효과 적용
	float data[2];
};

struct CostumeData {
	int index;
	int subCharacter;				// 어떤 캐릭터에 대한 코스튬인지
	int price;					// 값(보석)
};

class Obstacle;
class Gimmick;
class Temple;
class TCPClient;

struct MatchingData
{
	MATCHING_TYPE type;
	bool valid;				// 유효한 매칭 데이터인지 (csv 로딩 문제)

	int frame_rate;
	float crt_radius;
	float obstacle_radius;

	Rect map_size;
	int map_block_width;
	int map_block_height;
	int playtime;			// 제한 시간(게임 종료 판정 시간)
	int finish_score;		// 게임 종료 판정 점수

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

	Vector2 blockToField(Vector2 blockPos);// 블록 좌표를 매칭 필드 좌표로
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