#include "Player.h"
#include "Skill.h"
#include "Item.h"
#include "PlayerState.h"
#include "Matching.h"
#include "DebugClientManager.h"
#include "DataBase.h"
void Character::addHP(float delta)
{
	hp += delta;
	if (hp < 0)	hp = 0;
	if (hp > getMaxHP())
		hp = getMaxHP();
}
void Character::applyDamage(float damage, bool _cancelCasting)
{
	if (player->getState() == PLAYER_STATE::DEATH)
		return;

	for (int i = 0; i < player->crtCount; i++) {
		if (player->crtList[i].index != index) continue;
		hp -= damage;
		if (hp <= 0) {
			player->removeCharacter(i);

			if (i == 0)
			{
				player->ability.setSkillGauge(0);
				player->ability.setStamina(0);
				player->setState(PLAYER_STATE::DEATH);
			}
			else {
				player->sendPlayerDataToMe(PP_SPEED);
				player->sendPlayerDataToEnemy(PP_SPEED);
			}
		}
		if (i == 0 && _cancelCasting) {// 리더
			player->cancelCasting();// 캐스팅 취소
		}
		break;
	}
}
float Character::getMaxHP() {
	if (index == 0)
		return player->leader->maxHP
				+ player->ability.templeHp;
	else
		return sub->maxHP
				+player->ability.templeHp;
}
float Character::getDamage() {
	if (index == 0)
		return player->leader->damage
				+ player->ability.templeDamage;
	else
		return sub->damage
				+ player->ability.templeDamage;
}
float Ability::getSpeed() {
	if (runSkill || lockedInBubble || stun > 0)
		return 0;
	else if (allurement)// 매혹 효과
		return db_item[ITEM_TYPE::ALLUREMENT].data[4];
	else
	{
		float value = fixedSpeed + (4 - player->crtCount) * player->leader->increaseSpeed;
		float percent = speedPer;
		float additionalValue = marshSpeed;

		return value * percent + additionalValue;
	}
}
float Ability::getMaxStamina() {
	float value = fixedMaxStamina;

	return value;
}
float Ability::getMaxSkillGauge() {
	return player->leader->maxSkillGauge;
}
float Ability::getSkillDamage(float skillDamage) {
	return (skillDamage + templeDamage) * 1 + 0;
}
float Ability::getStaminaDecrementOfAttack() {
	return player->leader->decrementStaminaAttack;
}
float Ability::getStaminaDecrementOfDefense() {
	return player->leader->decrementStaminaDefense;
}
float Ability::getStaminaDecrementOfCasting() {
	return player->leader->decrementStaminaCasting;
}
float Ability::getRespawnTime() {
	return player->leader->respawnDelay;
}
float Ability::getStaminaRecoveryPower()
{
	return player->leader->recoveryStamina;
}
float Ability::getSkilGaugeRecoveryPower()
{
	return player->leader->recoverySkillGauge;
}
float Ability::getHpRecoveryPower()
{
	return templeRecovery;
}
float Ability::getAttackRecovery()
{
	return player->leader->attackRecovery;
}
void Ability::addSkillGauge(float gauge)
{
	skillGauge += gauge;
	if (skillGauge < 0)	skillGauge = 0;
	if (skillGauge > getMaxSkillGauge())
		skillGauge = getMaxSkillGauge();
}
void Ability::addStamina(float _stamina)
{
	stamina += _stamina;
	if (stamina < 0)	stamina = 0;
	if (stamina > getMaxStamina())
		stamina = getMaxStamina();
}
void Ability::setSkillGauge(float gauge)
{
	skillGauge = gauge;
	if (skillGauge < 0)	skillGauge = 0;
	if (skillGauge > getMaxSkillGauge())
		skillGauge = getMaxSkillGauge();
}
void Ability::setStamina(float _stamina)
{
	stamina = _stamina;
	if (stamina < 0)	stamina = 0;
	if (stamina > getMaxStamina())
		stamina = getMaxStamina();
}
float Ability::getStamina() {
	return stamina;
}
float Ability::getSkillGauge() {
	return skillGauge;
}
void Ability::calculateFixedStat()
{
	player->leader = &(db_leader);
	fixedSpeed = player->leader->speed;
	fixedMaxStamina = player->leader->maxStamina;

	UserCharacterData& data = player->userInfo->getUserData().character;
	for (int i = 0; i < 3; i++) {
		int sub_idx = data.subCharacterSlot[i];
		player->crtList[i + 1].sub = &(db_sub[sub_idx]);
		fixedSpeed += player->crtList[i + 1].sub->extraSpeed;
		fixedMaxStamina += player->crtList[i + 1].sub->extraStamina;
	}
}

Player::Player(User* _user, Matching* _matching)
{
	userInfo = _user;
	matching = _matching;
	ready = false;
	currentState = new MoveState();
	
	for (int i = 0; i < 3; i++) {
		skills[i] = nullptr;
		items[i] = nullptr;
	}
}
Player::~Player()
{
	for (int i = 0; i < 3; i++) {
		if (items[i] != nullptr)
			delete items[i];
		if (skills[i] != nullptr)
			delete skills[i];
	}
	if (currentState != nullptr)
	{
		delete currentState;
		currentState = nullptr;
	}
}
PLAYER_STATE Player::getState()
{
	matching->EnterCS();
	PLAYER_STATE s = currentState->getStateType();
	matching->LeaveCS();

	return s;
}
bool Player::isStateOf(PLAYER_STATE state)
{
	if (currentState == nullptr) {
		return false;
	}

	matching->EnterCS();
	bool b = (currentState->getStateType() == state);
	matching->LeaveCS();

	return b;
}
Player* Player::getEnemy()
{
	return matching->getEnemyOf(userInfo);
}
void Player::setState(PLAYER_STATE state, bool autoLogSend)
{
	matching->EnterCS();
	if (currentState != nullptr)
	{
		delete currentState;
		currentState = nullptr;
	}

	switch (state) {
	case PLAYER_STATE::MOVE:
		currentState = new MoveState();
		break;
	case PLAYER_STATE::ATTACK:
		currentState = new AttackState();
		break;
	case PLAYER_STATE::GUARD:
		currentState = new GuardState();
		break;
	case PLAYER_STATE::DEATH:
		currentState = new DeathState();
		break;
	}
	matching->LeaveCS();

	if (autoLogSend)
		DebugClientManager::GetInstance()->sendChangeState(this->matching, this->matching->getPlayer1() == this);
}
bool Player::attack(Player* player, int targetIndex)
{
	matching->EnterCS();
	bool b = currentState->attack(this, player, targetIndex);
	matching->LeaveCS();

	return b;
}
void Player::run()
{
	matching->EnterCS();
	currentState->run(this);
	matching->LeaveCS();
}
Character& Player::getCharacter(int id, bool& _out_found)
{
	for (int i = 0; i < crtCount; i++) {
		if (crtList[i].index == id && crtList[i].hp > 0) {
			_out_found = true;
			return crtList[i];
		}
	}
	_out_found = false;
	Character c;
	c.index = id;
	c.hp = 0;
	return c;
}
int Player::getCharacterIndex(int id) {
	for (int i = 0; i < crtCount; i++) {
		if (crtList[i].index == id && crtList[i].hp > 0) {
			return i;
		}
	}
	return -1;
}
void Player::applyDamage(int idx, float damage)
{
	crtList[idx].applyDamage(damage);
}
bool Player::canControl(int allow)
{
	// 조이스틱 조작, 전투 상태 변환, 스킬 사용, 캐스팅 사용 가능 여부
	bool runSkill = (IS_INCLUDE(allow, ALLOW_SKILL) ? true : ability.runSkill == false);
	bool bubble = ability.lockedInBubble == false;
	bool casting = (IS_INCLUDE(allow, ALLOW_CASTING) ? true : matching->getTemple().isCasting(this) == false);
	bool stun = ability.stun == 0;
	bool control = ability.ignoreControl == 0;
	return (runSkill && bubble && casting && stun && control);
}
bool Player::isCasting()
{
	return matching->getTemple().isCasting(this);
}
void Player::cancelCasting(bool initGauge)
{
	matching->EnterCS();
	if (isCasting())
	{
		Temple& temple = matching->getTemple();
		temple.tryToCasting(*this, *getEnemy(), false);

		// 게이지 초기화
		if (initGauge) {
			if (this == matching->getPlayer1() && temple.gauge > 0)
				temple.gauge = 0;
			if (this == matching->getPlayer2() && temple.gauge < 0)
				temple.gauge = 0;
		}

		SendPacket pk1(30), pk2(30);
		SendPacket pk3(10), pk4(10);
		pk1 << GAME_PROTOCOL::SERVER_TEMPLE_CASTING << true << false;
		pk1 << ability.getSpeed() << temple.gauge << isStateOf(PLAYER_STATE::DEATH);

		pk2 << GAME_PROTOCOL::SERVER_TEMPLE_CASTING << false << false;
		pk2 << ability.getSpeed() << temple.gauge << isStateOf(PLAYER_STATE::DEATH);

		userInfo->sendMsg(pk1);
		getEnemy()->userInfo->sendMsg(pk2);

		if (temple.allIntoTemple) {
			pk3 << GAME_PROTOCOL::SERVER_TEMPLE_ENTER << false << temple.gauge;
			pk4 << GAME_PROTOCOL::SERVER_TEMPLE_ENTER << false << temple.gauge;

			userInfo->sendMsg(pk3);
			getEnemy()->userInfo->sendMsg(pk4);
		}
	}
	matching->LeaveCS();

}
void Player::init()
{
	score = 0;
	ability.player = this;
	ability.templeDamage = 0;
	ability.templeHp = 0;
	ability.templeRecovery = 0;
	ability.calculateFixedStat();

	for (int idx = 0; idx < 3; idx++) {
		UserData& userdata = userInfo->getUserData();

		// skill
		int subIdx = userdata.character.subCharacterSlot[idx];
		if (subIdx != -1)
			skills[idx] = Skill::createSkill((SKILL_TYPE)db_sub[subIdx].skill, userInfo, matching);
		else
			skills[idx] = nullptr;

		// item
		int itemSlot = userdata.item.itemSlot[idx];
		if (itemSlot != -1)
			items[idx] = Item::createItem((ITEM_TYPE)itemSlot, userInfo, matching);
		else
			items[idx] = nullptr;
	}

	reset();
	// 최초 시작은 MAX
	ability.setSkillGauge(ability.getMaxSkillGauge());
	ability.setStamina(ability.getMaxStamina());

	sendPlayerDataToMe(PP_SKILLGAUGE | PP_STAMINA);
}
void Player::reset()
{
	/*
		Original code in 0927
	*/
	joystick = -999;
	ability.speedPer = 1;
	ability.marshSpeed = 0;
	ability.runSkill = false;
	ability.lockedInBubble = false;
	ability.stun = 0;
	ability.curse = 0;
	ability.ignoreControl = 0;
	ability.allurement = false;
	ability.setSkillGauge(0);
	ability.setStamina(0);
	setState(PLAYER_STATE::MOVE);

	bool isP1 = (matching->getPlayer1() == this);// Player1 ??

	crtCount = 4;
	crtList[0].position = (isP1 ? matching->getMatchingData().p1_startPosition : matching->getMatchingData().p2_startPosition);
	crtList[0].direction = (isP1 ? matching->getMatchingData().p1_startDirection : matching->getMatchingData().p2_startDirection);
	for (int i = 0; i < 4; i++) {
		crtList[i].index = i;
		crtList[i].player = this;
		crtList[i].hp = crtList[i].getMaxHP();
		crtList[i].bleedingDmg = 0;
		crtList[i].bleedingTime = 0;

		if (i == 0) continue;
		crtList[i].direction = crtList[0].direction;
		crtList[i].position = crtList[i - 1].position - crtList[0].direction * matching->getMatchingData().crt_radius * 2;
		crtList[i].subCharacterId = i - 1;// 수정 필요
	}
	currentDistance = 0;
	for (int i = 0; i < 6; i++) {
		footprints[i] = crtList[0].position - crtList[0].direction * 1 * i;
	}

	//sendPlayerDataToMe(PP_ALL_TO_ME - PP_POSITION);
	//sendPlayerDataToEnemy(PP_ALL_TO_ENEMY - PP_POSITION);
	sendPlayerDataToMe(PP_ALL_TO_ME);
	sendPlayerDataToEnemy(PP_ALL_TO_ENEMY);
}
void Player::removeCharacter(int index)
{
	if (index >= crtCount) {
		return;
	}
	for (int i = index; i < crtCount - 1; i++) {
		crtList[i] = crtList[i + 1];
	}
	crtCount--;
	if (index == 0)
		crtCount = 0;
}
float Player::calculateAngularVelocity(float playerAngle, float targetAngle)
{
	if (targetAngle == -999)	return 0;

	float deltaAngle = targetAngle - playerAngle;

	if (deltaAngle < -180) {
		deltaAngle += 360;
	}
	if (deltaAngle > 180) {
		deltaAngle -= 360;
	}

	float gap = abs(deltaAngle);
	if (gap == 0) {
		return 0;
	}
	deltaAngle = deltaAngle / gap;
	if (gap < ability.getSpeed() * 0.5) {
		return deltaAngle * gap;
	}
	else {
		return deltaAngle * ability.getSpeed() * 0.5;
	}
}

bool operator == (int n, PACKING_PLAYER pp) {// n이 pp를 포함하면 true
	return ((n & pp) != 0);
}
bool operator != (int n, PACKING_PLAYER pp) {// n이 pp를 포함하지 않으면 true
	return ((n & pp) == 0);
}
void Player::sendPlayerDataToMe(int bitmask, int protocol)
{
	sendPlayerData(protocol, bitmask, true);
}
void Player::sendPlayerDataToEnemy(int bitmask, int protocol)
{
	sendPlayerData(protocol, bitmask, false);
}
void Player::sendPlayerData(int protocol, int bitmask, bool forMe)
{
	SendPacket pk(20);
	pk << protocol << bitmask;
	for (int i = 0; i <= 31; i++) {
		PACKING_PLAYER p = (PACKING_PLAYER)(1 << i);

		if (bitmask != p) continue;
		if (!forMe) {
			if ((int)PP_ALL_TO_ENEMY != p) continue;
		}

		switch (p) {
		case PP_RESPAWN:				  pk << forMe << crtList[0].direction << crtList[0].position; break;
		case PP_POSITION:				  pk << forMe << crtList[0].direction << crtCount;
			for (int j = 0; j < crtCount; j++) pk << crtList[j].index << crtList[j].position; break;
		case PP_MAXHP_ALL:				  pk << forMe << crtCount;
			for (int j = 0; j < crtCount; j++) pk << crtList[j].index << crtList[j].getMaxHP(); break;
		case PP_HP_ALL:					  pk << forMe << crtCount;
			for (int j = 0; j < crtCount; j++) pk << crtList[j].index << crtList[j].hp; break;
		case PP_SPEED:					  pk << forMe << ability.getSpeed(); break;
		case PP_SKILLGAUGE:				  pk << ability.getSkillGauge(); break;
		case PP_MAXSTAMINA:				  pk << ability.getMaxStamina(); break;
		case PP_STAMINA:				  pk << ability.getStamina(); break;
		case PP_MAXSKILLGAUGE:			  pk << ability.getMaxSkillGauge(); break;
		case PP_DECREASE_ATTACK:		  pk << ability.getStaminaDecrementOfAttack(); break;
		case PP_DECREASE_GUARD:			  pk << ability.getStaminaDecrementOfDefense(); break;
		case PP_DECREASE_CASTING:		  pk << ability.getStaminaDecrementOfCasting(); break;
		case PP_RECOVERY_STAMINA:		  pk << ability.getStaminaRecoveryPower(); break;
		case PP_RECOVERY_SKILLGAUGE:	  pk << ability.getSkilGaugeRecoveryPower(); break;
		case PP_SCORE:					  pk << forMe << score; break;
		case PP_COOLTIME_SKILL:
			for (int j = 0; j < 3; j++)
				if (skills[j] == nullptr)
					pk << 0.0f;
				else
					pk << skills[j]->getCoolTime();
			break;
		case PP_COOLTIME_ITEM:			  
			for (int j = 0; j < 3; j++) 
				if (items[j] == nullptr)
					pk << 0.0f;
				else
					pk << items[j]->getCoolTime();
			break;
		case PP_RECOVERY_HP:			  pk << forMe << ability.getHpRecoveryPower(); break;
		case PP_BLEEDING:				  pk << forMe << crtCount;
			for (int j = 0; j < crtCount; j++)
				pk << crtList[j].index << (crtList[j].bleedingTime > 0) << crtList[j].bleedingDmg;
			break;
		case PP_STUN:					  pk << forMe << (ability.stun>0) << ability.getSpeed(); break;
		case PP_CURSE:					  pk << (ability.curse > 0); break;
		case PP_IGNORE_CONTROL:			  pk << (ability.ignoreControl > 0); break;
		default: i = 32; break;
		}
	}

	if (forMe)
		userInfo->sendMsg(pk);
	else
		getEnemy()->userInfo->sendMsg(pk);
}