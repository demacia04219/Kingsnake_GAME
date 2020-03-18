#include "PlayerState.h"
#include "Player.h"
#include "Global.h"
#include "GlobalProtocol.h"
#include "Matching.h"
#include "DataBase.h"

PLAYER_STATE AttackState::getStateType() { return PLAYER_STATE::ATTACK; }
PLAYER_STATE GuardState::getStateType() { return PLAYER_STATE::GUARD; }
PLAYER_STATE MoveState::getStateType() { return PLAYER_STATE::MOVE; }
PLAYER_STATE DeathState::getStateType() { return PLAYER_STATE::DEATH; }

/* PlayerState Section */
void PlayerState::rotateLeader(Player* _player)
{
	Vector2& vec = _player->crtList[0].direction;
	float rotation = Vector2::angle(vec);
	rotation += _player->calculateAngularVelocity(rotation, _player->joystick);
	vec.x = cos(rotation * 3.141592 / 180);
	vec.y = sin(rotation * 3.141592 / 180);
}
void PlayerState::moveLeader(Player* _player)
{
	float speedPerFrame = _player->ability.getSpeed() / _player->matching->getMatchingData().frame_rate;

	_player->crtList[0].position += _player->crtList[0].direction * speedPerFrame;
	_player->currentDistance += speedPerFrame;
}
bool PlayerState::isFootprintNeedChange(Player* _player)
{
	return (_player->currentDistance >= 1);
}
void PlayerState::changeFootprint(Player* _player)
{
	_player->currentDistance--;
	for (int i = 5; i > 0; i--) {
		_player->footprints[i] = _player->footprints[i - 1];
	}
	_player->footprints[0] = _player->crtList[0].position;
}
void PlayerState::moveSubs(Player* _player)
{
	float speedPerFrame = _player->ability.getSpeed() / _player->matching->getMatchingData().frame_rate;

	for (int i = 1; i < _player->crtCount; i++) {
		Vector2 direction = (_player->footprints[i * 2 - 1] - _player->crtList[i].position);
		float distance = direction.size();
		if (distance <= speedPerFrame / 2)
			continue;
		if (distance < 1.1f) {
			direction = direction.normalization();
			_player->crtList[i].position += direction * speedPerFrame;
		}
		else {
			_player->crtList[i].position += direction * 0.3;
		}
	}
}

/*  ==============
	attack Section
	============== */

/* GuardState Section */
bool GuardState::attack(Player* player, Player* target, int targetIndex)
{
	return false;
}

/* AttackState Section */
bool AttackState::attack(Player* player, Player* target, int targetIndex)
{
	if (target->isStateOf(PLAYER_STATE::GUARD) == false && target->isStateOf(PLAYER_STATE::DEATH) == false)
	{
		target->crtList[targetIndex].applyDamage(player->crtList[0].getDamage());
		return true;
	}
	else
		return false;
}

/* MoveState Section */
bool MoveState::attack(Player* player, Player* target, int targetIndex)
{
	return false;
}

/* DeathState Section */
bool DeathState::attack(Player* player, Player* target, int targetIndex)
{
	return false;
}

/*  ===========
	run Section
	=========== */

/* DeathState Section */
bool DeathState::isRespawnAvailable(Player* _player)
{
	return (respawnCount >= _player->ability.getRespawnTime() * _player->matching->getMatchingData().frame_rate);
}

void DeathState::run(Player* player)
{
	respawnCount++;

	run_base(player, getStateType());

	if (isRespawnAvailable(player))
	{
		player->reset();
		player->ability.speedPer = 1;
		player->setState(PLAYER_STATE::MOVE);
	}
}

/* MoveState Section */
void MoveState::run(Player* player) 
{
	rotateLeader(player);
	moveLeader(player);

	if (isFootprintNeedChange(player)) changeFootprint(player);// ������ ����
	
	moveSubs(player);

	run_base(player, getStateType());
}

/* AttackState Section */
void AttackState::run(Player* player)
{
	rotateLeader(player);  // ���� ���� ����
	moveLeader(player); // ���� �̵�

	if (isFootprintNeedChange(player)) changeFootprint(player);// ������ ����

	moveSubs(player); // ���� ĳ���� �̵�

	run_base(player, getStateType());
}

/* GuardState Section */
void GuardState::run(Player* player)
{
	run_base(player, getStateType());
}
void PlayerState::run_base(Player* player, PLAYER_STATE stat)
{
	MatchingData& md = player->matching->getMatchingData();

	if (stat != DEATH) {
		bool sendBleeding = false;
		
		// ĳ���ͺ� ó��
		for (int i = player->crtCount-1; i >= 0; i--) 
		{
			Character& c = player->crtList[i];
			c.addHP(player->ability.getHpRecoveryPower() / md.frame_rate);// ü�� ���� ȸ��
			
			// ���� ����
			if (c.bleedingTime > 0)
			{
				c.bleedingTime--;
				c.applyDamage(c.bleedingDmg / md.frame_rate, false);

				if (c.bleedingTime == 0) sendBleeding = true;
			}
		}
		// ���� ȿ�� ���� send
		if (sendBleeding) {
			player->sendPlayerDataToMe(PP_BLEEDING | PP_HP_ALL);
			player->sendPlayerDataToEnemy(PP_BLEEDING | PP_HP_ALL);
		}
		// ���̽�ƽ ���� �ݴ�
		if (player->ability.curse > 0) {
			player->ability.curse --;
			if (player->ability.curse == 0) 
				player->sendPlayerDataToMe(PP_CURSE);
		}
		// ���̽�ƽ ���� ����
		if (player->ability.ignoreControl > 0) {
			player->ability.ignoreControl--;
			if (player->ability.ignoreControl == 0)
				player->sendPlayerDataToMe(PP_IGNORE_CONTROL);
		}

		player->ability.addSkillGauge(player->ability.getSkilGaugeRecoveryPower() / md.frame_rate);
	}
	if (stat == ATTACK || stat == GUARD) {
		if (player->ability.getStamina() <= 0) {
			player->ability.speedPer = 1;
			player->setState(PLAYER_STATE::MOVE);
			SendPacket pk1(20), pk2(20);
			pk1 << GAME_PROTOCOL::SERVER_ACTIVE_MOVE << true << player->ability.getSpeed();
			pk2 << GAME_PROTOCOL::SERVER_ACTIVE_MOVE << false << player->ability.getSpeed();

			player->userInfo->sendMsg(pk1);
			player->getEnemy()->userInfo->sendMsg(pk2);
			player->sendPlayerDataToMe(PP_STAMINA);
		}

	}

	switch (stat)
	{
	case MOVE:
		if (player->ability.stun > 0) {
			player->ability.stun--;
			if (player->ability.stun == 0) {
				player->sendPlayerDataToMe(PP_STUN);
				player->sendPlayerDataToEnemy(PP_STUN);
			}
		}
		if (player->matching->getTemple().isCasting(player) == false)
			player->ability.addStamina(player->ability.getStaminaRecoveryPower() / md.frame_rate);
		break;
	case ATTACK:
		player->ability.addStamina(-player->ability.getStaminaDecrementOfAttack() / player->matching->getMatchingData().frame_rate);
		break;
	case GUARD:
		player->ability.addStamina(-player->ability.getStaminaDecrementOfDefense() / player->matching->getMatchingData().frame_rate);
		break;
	case DEATH:
		player->ability.setStamina(0);
		player->ability.setSkillGauge(0);
		break;
	default:
		break;
	}
}