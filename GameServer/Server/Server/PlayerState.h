#ifndef __PLAYER_STATE_H__
#define __PLAYER_STATE_H__

#include "Global.h"
class Player;

class PlayerState abstract
{
protected:
	void rotateLeader(Player*); // 리더 방향 변경
	void moveLeader(Player*); // 리더 이동
	bool isFootprintNeedChange(Player*); // 발자취 수정 여부 체크
	void changeFootprint(Player*); // 발자취 수정
	void moveSubs(Player*); // 서브 캐릭터 이동
	void run_base(Player* player, PLAYER_STATE stat);
public:
	virtual PLAYER_STATE getStateType() = 0;
	virtual bool attack(Player* player, Player* target, int targetIndex) = 0;// 일반 공격
	virtual void run(Player* player) = 0;	// 프레임 처리
};

class GuardState : public PlayerState
{
public:
	PLAYER_STATE getStateType() override;
	bool attack(Player* player, Player* target, int targetIndex) override;
	void run(Player* player) override;
};
class AttackState : public PlayerState
{
public:
	PLAYER_STATE getStateType() override;
	bool attack(Player* player, Player* target, int targetIndex) override;
	void run(Player* player) override;
};
class MoveState : public PlayerState
{
public:
	PLAYER_STATE getStateType() override;
	bool attack(Player* player, Player* target, int targetIndex) override;
	void run(Player* player) override;
};
class DeathState : public PlayerState
{
private:
	int respawnCount;

	bool isRespawnAvailable(Player*); // 리스폰 가능 여부 체크
public:
	DeathState() : respawnCount(0) {}
	PLAYER_STATE getStateType() override;
	bool attack(Player* player, Player* target, int targetIndex) override;
	void run(Player* player) override;
};

#endif