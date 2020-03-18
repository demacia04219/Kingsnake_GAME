#ifndef __PLAYER_STATE_H__
#define __PLAYER_STATE_H__

#include "Global.h"
class Player;

class PlayerState abstract
{
protected:
	void rotateLeader(Player*); // ���� ���� ����
	void moveLeader(Player*); // ���� �̵�
	bool isFootprintNeedChange(Player*); // ������ ���� ���� üũ
	void changeFootprint(Player*); // ������ ����
	void moveSubs(Player*); // ���� ĳ���� �̵�
	void run_base(Player* player, PLAYER_STATE stat);
public:
	virtual PLAYER_STATE getStateType() = 0;
	virtual bool attack(Player* player, Player* target, int targetIndex) = 0;// �Ϲ� ����
	virtual void run(Player* player) = 0;	// ������ ó��
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

	bool isRespawnAvailable(Player*); // ������ ���� ���� üũ
public:
	DeathState() : respawnCount(0) {}
	PLAYER_STATE getStateType() override;
	bool attack(Player* player, Player* target, int targetIndex) override;
	void run(Player* player) override;
};

#endif