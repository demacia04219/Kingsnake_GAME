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
	int index;			// ������ȣ
	int subCharacterId;	// ĳ����
	Vector2 direction;	// ����
	Vector2 position;	// ��ġ
	float hp;
	float getMaxHP();
	float getDamage();	// �Ϲ� ���� or �ڵ� ����
	void applyDamage(float damage, bool _cancelCasting=true);
	void addHP(float delta);

	// ���� ����
	int bleedingTime;	// ���� �ð�(���� ������). ��Ȱ�� ���´� 0. state���� �ڵ����� ���ҽ�Ŵ
	float bleedingDmg;	// ���� ������(�ʴ�)

	/*���� ������ ����. Ability::calculateFixedStat()�� �ʱ�ȭ*/
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
	float skillGauge;						// ���� ������
	float stamina;							// ���� ���׹̳�

public:
	// ���� ����
	float getSkillGauge();					// ���� ������
	float getStamina();						// ���� ���׹̳�
	float getSpeed();						// ���� ���ǵ�
	float getMaxStamina();					// �ִ� ���׹̳�
	float getMaxSkillGauge();				// �ִ� ��ų ������
	float getStaminaRecoveryPower();		// ���׹̳� ȸ����(��)
	float getSkilGaugeRecoveryPower();		// ��ų ������ ȸ����(��)
	float getHpRecoveryPower();				// ü�� ȸ����(��)
	float getStaminaDecrementOfAttack();	// ���� ���׹̳� ���ҷ�(�ʴ�)
	float getStaminaDecrementOfDefense();	// ��� ���׹̳� ���ҷ�(�ʴ�)
	float getStaminaDecrementOfCasting();	// ĳ���� ���׹̳� ���ҷ�(�ʴ�)
	float getRespawnTime();					// ��Ȱ �ð�(��)
	float getAttackRecovery();				// ���� ���� �� ��ų ������ ȸ��
	float getSkillDamage(float skillDamage);// ���� ��ų ������

	// set �Լ�
	void setSkillGauge(float gauge);
	void setStamina(float stamina);
	void addSkillGauge(float gauge);
	void addStamina(float stamina);

	// ��� ���� ����(���� ����)
	float speedPer;		// ĳ�� ���¿� ���� �ӵ� ��ȭ(%)
	bool runSkill;		// ��ų ���� ��?. �̼� 0
	float templeDamage;	// ���� ȿ��(���ݷ�)
	float templeRecovery;// ���� ȿ��(ü�� ���� ȸ��)
	float templeHp;		// ���� ȿ��(�ִ� ü��)
	float marshSpeed;	// �� �̼� ���� ȿ��
	bool lockedInBubble;// ����� ����
	int stun;			// ����(���� ����, �ӵ� 0). ��Ȱ�� ���´� 0. state���� �ڵ����� ���ҽ�Ŵ
	int curse;			// ����(���̽�ƽ �ݴ�) �ð�(���� ������). ��Ȱ�� ���´� 0. state���� �ڵ����� ���ҽ�Ŵ
	int ignoreControl;	// ���� ����(���� ������). ��Ȱ�� ���´� 0. state���� �ڵ����� ���ҽ�Ŵ
	bool allurement;	// ��Ȥ ��ų(��Ȥ�� �ɷ� �̼��� Ư�� ���� ��)

	// ��� ���� ����(���� ����: ���� 1ȸ ���)
	void calculateFixedStat();
	float fixedSpeed;
	float fixedMaxStamina;

	//
	Player* player;
};

// canControl() �μ�(��Ʈ����ũ)
#define		ALLOW_NULL		0				// ����� �׸� ����
#define		ALLOW_SKILL		(1 << 0)		// ��ų�� ��� ���̾ �÷��̾� ������ ���
#define		ALLOW_CASTING	(1 << 1)		// ĳ���� ���̾ �÷��̾� ������ ���
#define		IS_INCLUDE(source,item)	((source & item) != 0)// source�� item�� ���ԵǴ��� �˻�
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
	float joystick;			// ����
	int score;				// ����
	Ability ability;		// ĳ���� �ɷ�ġ
	Character crtList[4];
	Vector2 footprints[6];	// ���� ������
	Skill* skills[3];		// ��ų ����
	Item* items[3];			// ������ ����

	// func
	Player(User*, Matching*);
	~Player();
	void init();			// �ʱ�ȭ(���� �� ��)
	void reset();			// �׾��� ��(�κ� �ʱ�ȭ)

	Character& getCharacter(int id, bool& _out_found);
	int getCharacterIndex(int id);
	void removeCharacter(int index);
	Player* getEnemy();
	float calculateAngularVelocity(float playerAngle, float targetAngle);
	void applyDamage(int idx, float damage);
	bool canControl(int allow = ALLOW_NULL);	// ĳ���Ͱ� ���� ������ ��������. �μ��� ����� �׸�(��Ʈ����)
	bool isCasting();
	void cancelCasting(bool initGauge=true);

	User * userInfo;
	Matching* matching;
	void sendPlayerDataToMe(int bitmask, int protocol=SERVER_DATA_PLAYER);	// �� ������ ������
	void sendPlayerDataToEnemy(int bitmask, int protocol=SERVER_DATA_PLAYER);// �� ������ ������

private:
	void sendPlayerData(int protocol, int bitmask, bool forMe);

public:
	/*���� ������ ����. Ability::calculateFixedStat���� �ʱ�ȭ*/
	LeaderCharacterData* leader;
};

#endif
