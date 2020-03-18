#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "User.h"
#include "Global.h"
#include "Vector.h"
#include <vector>
#include "GlobalProtocol.h"
#include <map>

class Player;
class SubCharacterData;
class LeaderCharacterData;
struct Character
{
	int index;			// 고유번호
	int subCharacterId;	// 캐릭터
	Vector2 direction;	// 방향
	Vector2 position;	// 위치
	float hp;
	float getMaxHP();
	float getDamage();	// 일반 공격 or 자동 공격
	void applyDamage(float damage, bool _cancelCasting=true);
	void addHP(float delta);

	// 보조 스텟
	int bleedingTime;	// 출혈 시간(단위 프레임). 비활성 상태는 0. state에서 자동으로 감소시킴
	float bleedingDmg;	// 출혈 데미지(초당)

	/*빠른 접근을 위해. Ability::calculateFixedStat()로 초기화*/
	SubCharacterData* sub;

	//
	Player* player;
};

class PlayerState;
class Matching;

struct Skill;
struct Item;

struct Ability
{
private:
	float skillGauge;						// 현재 게이지
	float stamina;							// 현재 스테미나

public:
	// 실제 스텟
	float getSkillGauge();					// 현재 게이지
	float getStamina();						// 현제 스테미나
	float getSpeed();						// 현재 스피드
	float getMaxStamina();					// 최대 스테미나
	float getMaxSkillGauge();				// 최대 스킬 게이지
	float getStaminaRecoveryPower();		// 스테미나 회복량(초)
	float getSkilGaugeRecoveryPower();		// 스킬 게이지 회복량(초)
	float getHpRecoveryPower();				// 체력 회복량(초)
	float getStaminaDecrementOfAttack();	// 공격 스테미나 감소량(초당)
	float getStaminaDecrementOfDefense();	// 방어 스테미나 감소량(초당)
	float getStaminaDecrementOfCasting();	// 캐스팅 스테미나 감소량(초당)
	float getRespawnTime();					// 부활 시간(초)
	float getAttackRecovery();				// 공격 성공 시 스킬 게이지 회복
	float getSkillDamage(float skillDamage);// 최종 스킬 데미지

	// set 함수
	void setSkillGauge(float gauge);
	void setStamina(float stamina);
	void addSkillGauge(float gauge);
	void addStamina(float stamina);

	// 계산 보조 스텟(유동 스텟)
	float speedPer;		// 캐릭 상태에 따른 속도 변화(%)
	bool runSkill;		// 스킬 시전 중?. 이속 0
	float templeDamage;	// 제단 효과(공격력)
	float templeRecovery;// 제단 효과(체력 지속 회복)
	float templeHp;		// 제단 효과(최대 체력)
	float marshSpeed;	// 늪 이속 감소 효과
	bool lockedInBubble;// 물방울 갇힘
	int stun;			// 기절(조작 무시, 속도 0). 비활성 상태는 0. state에서 자동으로 감소시킴
	int curse;			// 저주(조이스틱 반대) 시간(단위 프레임). 비활성 상태는 0. state에서 자동으로 감소시킴
	int ignoreControl;	// 조작 무시(단위 프레임). 비활성 상태는 0. state에서 자동으로 감소시킴
	bool allurement;	// 매혹 스킬(매혹에 걸려 이속이 특정 값이 됨)

	// 계산 보조 스텟(고정 스텟: 최초 1회 계산)
	void calculateFixedStat();
	float fixedSpeed;
	float fixedMaxStamina;

	//
	Player* player;
};

// canControl() 인수(비트마스크)
#define		ALLOW_NULL		0				// 허용할 항목 없음
#define		ALLOW_SKILL		(1 << 0)		// 스킬이 사용 중이어도 플레이어 조작을 허용
#define		ALLOW_CASTING	(1 << 1)		// 캐스팅 중이어도 플레이어 조작을 허용
#define		IS_INCLUDE(source,item)	((source & item) != 0)// source에 item이 포함되는지 검사
//

struct Player
{
	// state
	PlayerState* currentState;
	void setState(PLAYER_STATE state, bool autoLogSend=true);
	bool isStateOf(PLAYER_STATE state);
	PLAYER_STATE getState();
	bool attack(Player* player, int targetIndex);
	void run();

	// player stat
	bool ready;
	int crtCount;
	float currentDistance;	
	float joystick;			// 방향
	int score;				// 점수
	Ability ability;		// 캐릭터 능력치
	Character crtList[4];
	Vector2 footprints[6];	// 선두 발자취
	Skill* skills[3];		// 스킬 슬롯
	Item* items[3];			// 아이템 슬롯

	// func
	Player(User*, Matching*);
	~Player();
	void init();			// 초기화(최초 한 번)
	void reset();			// 죽었을 때(부분 초기화)

	Character& getCharacter(int id, bool& _out_found);
	int getCharacterIndex(int id);
	void removeCharacter(int index);
	Player* getEnemy();
	float calculateAngularVelocity(float playerAngle, float targetAngle);
	void applyDamage(int idx, float damage);
	bool canControl(int allow = ALLOW_NULL);	// 캐릭터가 조작 가능한 상태인지. 인수는 허용할 항목(비트연산)
	bool isCasting();
	void cancelCasting(bool initGauge=true);

	User * userInfo;
	Matching* matching;
	void sendPlayerDataToMe(int bitmask, int protocol=SERVER_DATA_PLAYER);	// 내 정보를 나에게
	void sendPlayerDataToEnemy(int bitmask, int protocol=SERVER_DATA_PLAYER);// 내 정보를 적에게

private:
	void sendPlayerData(int protocol, int bitmask, bool forMe);

public:
	/*빠른 접근을 위해. Ability::calculateFixedStat에서 초기화*/
	LeaderCharacterData* leader;
};

#endif
