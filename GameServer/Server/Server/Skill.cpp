#include "Skill.h"
#include "User.h"
#include "Matching.h"
#include "StaticGameData.h"
#include "DataBase.h"
#include "GameObject.h"
#include <map>

Skill::Skill(User* u, Matching* m, SKILL_TYPE type) :user(*u), matching(*m), coolTime(0), runTime(0)
{
	this->type = type;
	skillData = &(db_skill[type]);
}
int Skill::getCoolFrame() {
	return coolTime;
}
float Skill::getCoolTime() { 
	return ((float)coolTime) / matching.getMatchingData().frame_rate;
}
int Skill::getSlotIndex()
{
	Player& p = *(matching.getPlayerOf(&user));
	for (int i = 0; i < 3; i++) {
		if (p.skills[i] == this) {
			return i;
		}
	}
	return -1;
}
Skill* Skill::createSkill(SKILL_TYPE idx, User* user, Matching* matching)
{
	switch (idx)
	{
	case SHADOWSWORD:
		return new ShadowSword(user, matching);
	case WINDPUSH:
		return new WindPush(user, matching);
	case BUBBLETRAP:
		return new BubbleTrap(user, matching);
	}
	return nullptr;
}
BubbleTrap::BubbleTrap(User* u, Matching* m) : Skill(u, m, BUBBLETRAP) {
}
BubbleTrap::~BubbleTrap() {

}
void BubbleTrap::recv() {
	int step;
	user >> step;

	SendPacket pk1(30), pk2(30);
	SendPacket pk3(40);
	bool send = false;

	matching.EnterCS();
	Player* me = matching.getPlayerOf(&user);
	Player* enemy = me->getEnemy();

	switch (step)
	{
	case 0:	// 스킬 활성화
		if (coolTime == 0 && me->ability.getSkillGauge() >= skillData->gaugeCost &&
			me->canControl() && me->getState() == PLAYER_STATE::MOVE) {
			coolTime = skillData->coolTime * matching.getMatchingData().frame_rate;
			runTime = 0;
			send = true;

			me->ability.runSkill = true;
			me->ability.addSkillGauge(-skillData->gaugeCost);
			pk1 << GAME_PROTOCOL::SERVER_SKILL << true << getSlotIndex() << 1 << me->ability.getSkillGauge() << me->ability.getSpeed();
			pk2 << GAME_PROTOCOL::SERVER_SKILL << false << getSlotIndex() << 1 << me->ability.getSkillGauge() << me->ability.getSpeed();

			Bubble* obj = new Bubble(&user, &matching);
			obj->position = me->crtList[0].position + me->crtList[0].direction * matching.getMatchingData().crt_radius;
			obj->direction = me->crtList[0].direction;
			matching.addRuns(obj);

			pk3 << GAME_PROTOCOL::SERVER_GAMEOBJECT << obj->getSerial() << 3 << obj->getType() << obj->position << obj->direction;
		}
		break;
	}

	matching.LeaveCS();

	if (send) {
		me->userInfo->sendMsg(pk1);
		me->userInfo->sendMsg(pk3);
		enemy->userInfo->sendMsg(pk2);
		enemy->userInfo->sendMsg(pk3);
	}
}
void BubbleTrap::process() {
	std::map<SendPacket*, User*> sendPacket;

	matching.EnterCS();
	Player* me = matching.getPlayerOf(&user);
	Player* enemy = me->getEnemy();

	coolTime--;
	runTime++;

	// 스킬 시전 끝
	if (runTime == (int)(skillData->runTime * matching.getMatchingData().frame_rate)) {
		me->ability.runSkill = false;	// 시전 끝
		SendPacket* p = new SendPacket(30);
		SendPacket* p2 = new SendPacket(30);
		*p << GAME_PROTOCOL::SERVER_SKILL << true << getSlotIndex() << 2 << me->ability.getSpeed();
		*p2 << GAME_PROTOCOL::SERVER_SKILL << false << getSlotIndex() << 2 << me->ability.getSpeed();
		sendPacket[p] = me->userInfo;
		sendPacket[p2] = enemy->userInfo;
	}

	matching.LeaveCS();

	while (sendPacket.size() > 0) {
		auto d = sendPacket.begin();
		SendPacket* p = d->first;
		d->second->sendMsg(*p);
		sendPacket.erase(p);
		delete p;
	}
}
bool BubbleTrap::isActived() {
	return (coolTime > 0);
}

ShadowSword::ShadowSword(User* u, Matching* m) : Skill(u, m, SHADOWSWORD) {

}
ShadowSword::~ShadowSword() {

}
void ShadowSword::recv() {
	int step;
	user >> step;
	
	SendPacket packA_1(100), packA_2(100);
	bool sendPackA = false;

	matching.EnterCS();
	Player* me = matching.getPlayerOf(&user);
	Player* enemy = me->getEnemy();

	switch (step) {
	case 0:	// 스킬 활성화
		if (coolTime == 0 && me->ability.getSkillGauge() >= skillData->gaugeCost &&
			me->canControl() && me->getState() == PLAYER_STATE::MOVE) {
			me->ability.runSkill = true;
			me->ability.addSkillGauge(-skillData->gaugeCost);
			packA_1 << GAME_PROTOCOL::SERVER_SKILL << true << getSlotIndex() << 1 << me->ability.getSkillGauge() << me->ability.getSpeed();
			packA_2 << GAME_PROTOCOL::SERVER_SKILL << false << getSlotIndex() << 1 << me->ability.getSkillGauge() << me->ability.getSpeed();
			coolTime = skillData->coolTime * matching.getMatchingData().frame_rate;
			runTime = 0;
			sendPackA = true;
		}
		break;
	}

	matching.LeaveCS();

	if (sendPackA) {
		me->userInfo->sendMsg(packA_1);
		enemy->userInfo->sendMsg(packA_2);
	}
}
void ShadowSword::process() {
	std::map<SendPacket*, User*> sendPacket;

	matching.EnterCS();

	coolTime--;
	runTime++;

	Player* me = matching.getPlayerOf(&user);
	Player* enemy = me->getEnemy();


	// 스킬 시전 끝
	if (runTime == (int)(skillData->runTime * matching.getMatchingData().frame_rate)) {
		me->ability.runSkill = false;	// 시전 끝
		SendPacket* p = new SendPacket(30);
		SendPacket* p2 = new SendPacket(30);
		*p << GAME_PROTOCOL::SERVER_SKILL << true << getSlotIndex() << 2 << me->ability.getSpeed();
		*p2 << GAME_PROTOCOL::SERVER_SKILL << false << getSlotIndex() << 2 << me->ability.getSpeed();
		sendPacket[p] = me->userInfo;
		sendPacket[p2] = enemy->userInfo;
	}
	// 투사체 날림
	if (runTime == (int)(skillData->data[2] * matching.getMatchingData().frame_rate)
		&& me->isStateOf(PLAYER_STATE::DEATH) == false) {
		Sword* obj = new Sword(&user, &matching);
		obj->position = me->crtList[0].position + me->crtList[0].direction * matching.getMatchingData().crt_radius;
		obj->direction = me->crtList[0].direction;
		matching.addRuns(obj);

		SendPacket* p = new SendPacket(50);
		SendPacket* p2 = new SendPacket(50);
		*p << GAME_PROTOCOL::SERVER_GAMEOBJECT << obj->getSerial() << 3 << obj->getType() << obj->position << obj->direction;
		*p2 << GAME_PROTOCOL::SERVER_GAMEOBJECT << obj->getSerial() << 3 << obj->getType() << obj->position << obj->direction;
		sendPacket[p] = me->userInfo;
		sendPacket[p2] = enemy->userInfo;
	}

	matching.LeaveCS();

	while (sendPacket.size() > 0) {
		auto d = sendPacket.begin();
		SendPacket* p = d->first;
		d->second->sendMsg(*p);
		sendPacket.erase(p);
		delete p;
	}
}
bool ShadowSword::isActived() {
	return (coolTime > 0);
}

WindPush::WindPush(User* u, Matching* m) : Skill(u, m, WINDPUSH) {
}
WindPush::~WindPush() {

}
void WindPush::recv() {
	int step;
	user >> step;

	SendPacket pk1(50), pk2(50);
	SendPacket pk3(50), pk4(50);
	bool send = false;
	bool hit = false;

	matching.EnterCS();
	Player* me = matching.getPlayerOf(&user);
	Player* enemy = me->getEnemy();

	switch (step)
	{
	case 0:
		if (coolTime == 0 && me->ability.getSkillGauge() >= skillData->gaugeCost &&
			me->canControl() && me->getState() == PLAYER_STATE::MOVE) {
			me->ability.runSkill = true;
			me->ability.addSkillGauge(-skillData->gaugeCost);
			pk1 << GAME_PROTOCOL::SERVER_SKILL << true << getSlotIndex() << 1 << me->ability.getSkillGauge() << me->ability.getSpeed();
			pk2 << GAME_PROTOCOL::SERVER_SKILL << false << getSlotIndex() << 1 << me->ability.getSkillGauge() << me->ability.getSpeed();
			coolTime = skillData->coolTime * matching.getMatchingData().frame_rate;
			runTime = 0;
			send = true;

			hit = matching.Action_hitTest_circle(enemy, me->crtList[0].position, skillData->data[0]);
			if (hit) {
				float distance = (enemy->crtList[0].position - me->crtList[0].position).size();
				matching.Action_reflection_circle(enemy, me->crtList[0].position, distance+1);
				enemy->crtList[0].applyDamage(me->ability.getSkillDamage(skillData->data[1]));
				me->ability.addSkillGauge(me->ability.getAttackRecovery());
				bool found;
				Character& c = enemy->getCharacter(0, found);

				pk3 << GAME_PROTOCOL::SERVER_SKILL << false << getSlotIndex() << 10 << c.hp << c.position << c.direction;
				pk4 << GAME_PROTOCOL::SERVER_SKILL << true << getSlotIndex() << 10 << c.hp << c.position << c.direction;
				me->sendPlayerDataToMe(PP_SKILLGAUGE);
			}
		}
		break;
	}

	matching.LeaveCS();

	if (send) {
		me->userInfo->sendMsg(pk1);
		enemy->userInfo->sendMsg(pk2);
	}
	if (hit) {
		me->userInfo->sendMsg(pk3);
		enemy->userInfo->sendMsg(pk4);
	}
}
void WindPush::process() {
	std::map<SendPacket*, User*> sendPacket;

	matching.EnterCS();
	Player* me = matching.getPlayerOf(&user);
	Player* enemy = me->getEnemy();

	coolTime--;
	runTime++;

	// 스킬 시전 끝
	if (runTime == (int)(skillData->runTime * matching.getMatchingData().frame_rate)) {
		me->ability.runSkill = false;	// 시전 끝
		SendPacket* p = new SendPacket(30);
		SendPacket* p2 = new SendPacket(30);
		*p << GAME_PROTOCOL::SERVER_SKILL << true << getSlotIndex() << 2 << me->ability.getSpeed();
		*p2 << GAME_PROTOCOL::SERVER_SKILL << false << getSlotIndex() << 2 << me->ability.getSpeed();
		sendPacket[p] = me->userInfo;
		sendPacket[p2] = enemy->userInfo;
	}

	matching.LeaveCS();

	while (sendPacket.size() > 0) {
		auto d = sendPacket.begin();
		SendPacket* p = d->first;
		d->second->sendMsg(*p);
		sendPacket.erase(p);
		delete p;
	}
}
bool WindPush::isActived() {
	return (coolTime > 0);
}