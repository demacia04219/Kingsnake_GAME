#include "Item.h"
#include "User.h"
#include "Matching.h"
#include "StaticGameData.h"
#include "DataBase.h"
#include "GameObject.h"

Item::Item(User* u, Matching* m, ITEM_TYPE type) : user(*u), matching(*m), coolTime(0), runTime(0)
{
	this->type = type;
	itemData = &(db_item[type]);
}
int Item::getSlotIndex()
{
	Player& p = *(matching.getPlayerOf(&user));
	for (int i = 0; i < 3; i++) {
		if (p.items[i] == this) {
			return i;
		}
	}
	return -1;
}
int Item::getCoolFrame() { 
	return coolTime; 
}
float Item::getCoolTime() { 
	return ((float)coolTime) / matching.getMatchingData().frame_rate;
}
Item* Item::createItem(ITEM_TYPE idx, User* u, Matching* m)
{
	switch (idx)
	{
	case RABBIT:		return new Rabbit(u,m);
	case ARROWSNIPE:	return new ArrowSnipe(u, m);
	case SCRATCH:		return new Scratch(u, m);
	case NIGHTHOLLOW:	return new NightHollow(u, m);
	case CURSE:			return new Curse(u, m);
	case TELEPORT:		return new Teleport(u, m);
	case ALLUREMENT:	return new Allurement(u, m);
	}
	return nullptr;
}






Rabbit::Rabbit(User* u, Matching* m) :Item(u, m, RABBIT)
{
	bomb = nullptr;
}
void Rabbit::recv() {
	int step;
	user >> step;

	SendPacket pk1(30), pk2(30);

	matching.EnterCS();

	if (step == 0) {// 버튼 누름
		Player* me = matching.getPlayerOf(&user);
		Player* enemy = me->getEnemy();

		if (coolTime == 0 && me->ability.getSkillGauge() >= itemData->gaugeCost &&
			me->canControl() && me->getState() == PLAYER_STATE::MOVE) {
			coolTime = itemData->coolTime * matching.getMatchingData().frame_rate;
			runTime = 0;

			me->ability.runSkill = true;
			me->ability.addSkillGauge(-itemData->gaugeCost);
			pk1 << GAME_PROTOCOL::SERVER_ITEMSKILL << true << getSlotIndex() << 1 << me->ability.getSkillGauge() << me->ability.getSpeed();
			pk2 << GAME_PROTOCOL::SERVER_ITEMSKILL << false << getSlotIndex() << 1 << me->ability.getSkillGauge() << me->ability.getSpeed();
			me->userInfo->sendMsg(pk1);
			enemy->userInfo->sendMsg(pk2);

			RabbitBomb* obj = new RabbitBomb(&user, &matching, this);
			obj->position = me->crtList[0].position + me->crtList[0].direction * matching.getMatchingData().crt_radius;
			obj->direction = me->crtList[0].direction;
			matching.addRuns(obj);
			bomb = obj;

			pk1.initBuffer();
			pk1 << GAME_PROTOCOL::SERVER_GAMEOBJECT << obj->getSerial() << 3 << obj->getType() << obj->position << obj->direction;
			me->userInfo->sendMsg(pk1);
			enemy->userInfo->sendMsg(pk1);
		}
	}
	else if (step == 11) {// 투사체 활성화
		if (bomb != nullptr) {
			bomb->explosion();
		}
	}

	matching.LeaveCS();
}
void Rabbit::process() {
	matching.EnterCS();

	Player* me = matching.getPlayerOf(&user);
	Player* enemy = me->getEnemy();

	coolTime--;
	runTime++;

	// 스킬 시전 끝
	if (runTime == (int)(itemData->runTime * matching.getMatchingData().frame_rate)) {
		me->ability.runSkill = false;	// 시전 끝
		SendPacket p(30);
		SendPacket p2(30);
		p << GAME_PROTOCOL::SERVER_ITEMSKILL << true << getSlotIndex() << 2 << me->ability.getSpeed();
		p2 << GAME_PROTOCOL::SERVER_ITEMSKILL << false << getSlotIndex() << 2 << me->ability.getSpeed();
		me->userInfo->sendMsg(p);
		enemy->userInfo->sendMsg(p2);
	}

	matching.LeaveCS();
}
bool Rabbit::isActived() {
	return (coolTime > 0);
}


ArrowSnipe::ArrowSnipe(User* u, Matching* m) :Item(u, m, ARROWSNIPE)
{
}
void ArrowSnipe::recv() {
	int step;
	user >> step;

	if (step != 0)	return;

	SendPacket pk1(30), pk2(30);

	matching.EnterCS();
	Player* me = matching.getPlayerOf(&user);
	Player* enemy = me->getEnemy();

	if (coolTime == 0 && me->ability.getSkillGauge() >= itemData->gaugeCost &&
		me->canControl() && me->getState() == PLAYER_STATE::MOVE) {
		coolTime = itemData->coolTime * matching.getMatchingData().frame_rate;
		runTime = 0;

		me->ability.runSkill = true;
		me->ability.addSkillGauge(-itemData->gaugeCost);
		pk1 << GAME_PROTOCOL::SERVER_ITEMSKILL << true << getSlotIndex() << 1 << me->ability.getSkillGauge() << me->ability.getSpeed();
		pk2 << GAME_PROTOCOL::SERVER_ITEMSKILL << false << getSlotIndex() << 1 << me->ability.getSkillGauge() << me->ability.getSpeed();
		me->userInfo->sendMsg(pk1);
		enemy->userInfo->sendMsg(pk2);

		Arrow* obj = new Arrow(&user, &matching);
		obj->position = me->crtList[0].position + me->crtList[0].direction * matching.getMatchingData().crt_radius;
		obj->direction = me->crtList[0].direction;
		matching.addRuns(obj);

		pk1.initBuffer();
		pk1 << GAME_PROTOCOL::SERVER_GAMEOBJECT << obj->getSerial() << 3 << obj->getType() << obj->position << obj->direction;
		me->userInfo->sendMsg(pk1);
		enemy->userInfo->sendMsg(pk1);
	}

	matching.LeaveCS();
}
void ArrowSnipe::process() {
	matching.EnterCS();

	Player* me = matching.getPlayerOf(&user);
	Player* enemy = me->getEnemy();

	coolTime--;
	runTime++;

	// 스킬 시전 끝
	if (runTime == (int)(itemData->runTime * matching.getMatchingData().frame_rate)) {
		me->ability.runSkill = false;	// 시전 끝
		SendPacket p(30);
		SendPacket p2(30);
		p << GAME_PROTOCOL::SERVER_ITEMSKILL << true << getSlotIndex() << 2 << me->ability.getSpeed();
		p2 << GAME_PROTOCOL::SERVER_ITEMSKILL << false << getSlotIndex() << 2 << me->ability.getSpeed();
		me->userInfo->sendMsg(p);
		enemy->userInfo->sendMsg(p2);
	}

	matching.LeaveCS();
}
bool ArrowSnipe::isActived() {
	return (coolTime > 0);
}


Scratch::Scratch(User* u, Matching* m) :Item(u, m, SCRATCH)
{
}
void Scratch::recv() {
	int step;
	user >> step;

	if (step != 0)	return;

	SendPacket pk1(30), pk2(30);

	matching.EnterCS();
	Player* me = matching.getPlayerOf(&user);
	Player* enemy = me->getEnemy();

	if (coolTime == 0 && me->ability.getSkillGauge() >= itemData->gaugeCost &&
		me->canControl() && me->getState() == PLAYER_STATE::MOVE) {
		coolTime = itemData->coolTime * matching.getMatchingData().frame_rate;
		runTime = 0;

		me->ability.runSkill = true;
		me->ability.addSkillGauge(-itemData->gaugeCost);
		pk1 << GAME_PROTOCOL::SERVER_ITEMSKILL << true << getSlotIndex() << 1 << me->ability.getSkillGauge() << me->ability.getSpeed();
		pk2 << GAME_PROTOCOL::SERVER_ITEMSKILL << false << getSlotIndex() << 1 << me->ability.getSkillGauge() << me->ability.getSpeed();
		me->userInfo->sendMsg(pk1);
		enemy->userInfo->sendMsg(pk2);

		bool send = false;
		Vector2 front = me->crtList[0].position + me->crtList[0].direction * itemData->data[D_HIT_RANGE];
		for (int i = enemy->crtCount - 1; i >= 0; i--)
		{
			bool hit = matching.Action_hitTest_circle(
				front, itemData->data[D_HIT_RANGE],
				enemy->crtList[i].position, matching.getMatchingData().crt_radius);
			if (hit) {
				// 출혈
				enemy->crtList[i].bleedingTime = itemData->data[D_DURATION];
				enemy->crtList[i].bleedingDmg = itemData->data[D_BLEEDING_DAMAGE];
				// 데미지
				enemy->crtList[i].applyDamage(enemy->ability.getSkillDamage(itemData->data[D_ATTACK_DAMAGE]));
				send = true;
			}
		}
		if (send) {
			me->ability.addSkillGauge(me->ability.getAttackRecovery());
			SendPacket sp(60), sp2(60);
			sp << SERVER_ITEMSKILL << false << getSlotIndex() << 15 << enemy->crtCount;
			sp2 << SERVER_ITEMSKILL << true << getSlotIndex() << 15 << enemy->crtCount;
			for (int i = 0; i < enemy->crtCount; i++)
			{
				sp << enemy->crtList[i].index << enemy->crtList[i].hp;
				sp2 << enemy->crtList[i].index << enemy->crtList[i].hp;
			}
			me->userInfo->sendMsg(sp);
			enemy->userInfo->sendMsg(sp2);

			me->sendPlayerDataToMe(PP_SKILLGAUGE);
			enemy->sendPlayerDataToEnemy(PP_BLEEDING);
			enemy->sendPlayerDataToMe(PP_BLEEDING);
		}
	}

	matching.LeaveCS();
}
void Scratch::process() {
	matching.EnterCS();

	Player* me = matching.getPlayerOf(&user);
	Player* enemy = me->getEnemy();

	coolTime--;
	runTime++;

	// 스킬 시전 끝
	if (runTime == (int)(itemData->runTime * matching.getMatchingData().frame_rate)) {
		me->ability.runSkill = false;	// 시전 끝
		SendPacket p(30);
		SendPacket p2(30);
		p << GAME_PROTOCOL::SERVER_ITEMSKILL << true << getSlotIndex() << 2 << me->ability.getSpeed();
		p2 << GAME_PROTOCOL::SERVER_ITEMSKILL << false << getSlotIndex() << 2 << me->ability.getSpeed();
		me->userInfo->sendMsg(p);
		enemy->userInfo->sendMsg(p2);
	}

	matching.LeaveCS();
}
bool Scratch::isActived() {
	return (coolTime > 0);
}


NightHollow::NightHollow(User* u, Matching* m) :Item(u, m, NIGHTHOLLOW)
{
}
void NightHollow::recv() {
	int step;
	user >> step;

	if (step != 0)	return;

	SendPacket pk1(30), pk2(30);

	matching.EnterCS();
	Player* me = matching.getPlayerOf(&user);
	Player* enemy = me->getEnemy();

	if (coolTime == 0 && me->ability.getSkillGauge() >= itemData->gaugeCost &&
		me->canControl() && me->getState() == PLAYER_STATE::MOVE) {
		coolTime = itemData->coolTime * matching.getMatchingData().frame_rate;
		runTime = 0;

		me->ability.runSkill = true;
		me->ability.addSkillGauge(-itemData->gaugeCost);
		pk1 << GAME_PROTOCOL::SERVER_ITEMSKILL << true << getSlotIndex() << 1 << me->ability.getSkillGauge() << me->ability.getSpeed();
		pk2 << GAME_PROTOCOL::SERVER_ITEMSKILL << false << getSlotIndex() << 1 << me->ability.getSkillGauge() << me->ability.getSpeed();
		me->userInfo->sendMsg(pk1);
		enemy->userInfo->sendMsg(pk2);

		BlackHole* obj = new BlackHole(&user, &matching);
		obj->position = me->crtList[0].position + me->crtList[0].direction * itemData->data[0];
		obj->direction = me->crtList[0].direction;
		matching.addRuns(obj);

		pk1.initBuffer();
		pk1 << GAME_PROTOCOL::SERVER_GAMEOBJECT << obj->getSerial() << 3 << obj->getType() << obj->position << obj->direction;
		me->userInfo->sendMsg(pk1);
		enemy->userInfo->sendMsg(pk1);
	}

	matching.LeaveCS();
}
void NightHollow::process() {
	matching.EnterCS();

	Player* me = matching.getPlayerOf(&user);
	Player* enemy = me->getEnemy();

	coolTime--;
	runTime++;

	// 스킬 시전 끝
	if (runTime == (int)(itemData->runTime * matching.getMatchingData().frame_rate)) {
		me->ability.runSkill = false;	// 시전 끝
		SendPacket p(30);
		SendPacket p2(30);
		p << GAME_PROTOCOL::SERVER_ITEMSKILL << true << getSlotIndex() << 2 << me->ability.getSpeed();
		p2 << GAME_PROTOCOL::SERVER_ITEMSKILL << false << getSlotIndex() << 2 << me->ability.getSpeed();
		me->userInfo->sendMsg(p);
		enemy->userInfo->sendMsg(p2);
	}

	matching.LeaveCS();
}
bool NightHollow::isActived() {
	return (coolTime > 0);
}


Curse::Curse(User* u, Matching* m) :Item(u, m, CURSE)
{
}
void Curse::recv() {
	int step;
	user >> step;

	if (step != 0)	return;

	SendPacket pk1(20), pk2(20);

	matching.EnterCS();
	Player* me = matching.getPlayerOf(&user);
	Player* enemy = me->getEnemy();

	if (coolTime == 0 && me->ability.getSkillGauge() >= itemData->gaugeCost &&
		me->canControl() && me->getState() == PLAYER_STATE::MOVE) {
		coolTime = itemData->coolTime * matching.getMatchingData().frame_rate;
		runTime = 0;

		me->ability.runSkill = true;
		me->ability.addSkillGauge(-itemData->gaugeCost);
		pk1 << GAME_PROTOCOL::SERVER_ITEMSKILL << true << getSlotIndex() << 1 << me->ability.getSkillGauge() << me->ability.getSpeed();
		pk2 << GAME_PROTOCOL::SERVER_ITEMSKILL << false << getSlotIndex() << 1 << me->ability.getSkillGauge() << me->ability.getSpeed();
		me->userInfo->sendMsg(pk1);
		enemy->userInfo->sendMsg(pk2);

		enemy->ability.curse = itemData->data[0] * matching.getMatchingData().frame_rate;
		enemy->sendPlayerDataToMe(PP_CURSE);
	}

	matching.LeaveCS();
}
void Curse::process() {
	matching.EnterCS();

	Player* me = matching.getPlayerOf(&user);
	Player* enemy = me->getEnemy();

	coolTime--;
	runTime++;

	// 스킬 시전 끝
	if (runTime == (int)(itemData->runTime * matching.getMatchingData().frame_rate)) {
		me->ability.runSkill = false;	// 시전 끝
		SendPacket p(30);
		SendPacket p2(30);
		p << GAME_PROTOCOL::SERVER_ITEMSKILL << true << getSlotIndex() << 2 << me->ability.getSpeed();
		p2 << GAME_PROTOCOL::SERVER_ITEMSKILL << false << getSlotIndex() << 2 << me->ability.getSpeed();
		me->userInfo->sendMsg(p);
		enemy->userInfo->sendMsg(p2);
	}

	matching.LeaveCS();
}
bool Curse::isActived() {
	return (coolTime > 0);
}


Teleport::Teleport(User* u, Matching* m) :Item(u, m, TELEPORT)
{
}
void Teleport::recv() {
	int step;
	user >> step;

	if (step != 0)	return;

	SendPacket pk1(30), pk2(30);

	matching.EnterCS();
	Player* me = matching.getPlayerOf(&user);
	Player* enemy = me->getEnemy();

	if (coolTime == 0 && me->ability.getSkillGauge() >= itemData->gaugeCost &&
		me->canControl() && me->getState() == PLAYER_STATE::MOVE) {
		coolTime = itemData->coolTime * matching.getMatchingData().frame_rate;
		runTime = 0;

		me->ability.runSkill = true;
		me->ability.addSkillGauge(-itemData->gaugeCost);
		pk1 << GAME_PROTOCOL::SERVER_ITEMSKILL << true << getSlotIndex() << 1 << me->ability.getSkillGauge() << me->ability.getSpeed();
		pk2 << GAME_PROTOCOL::SERVER_ITEMSKILL << false << getSlotIndex() << 1 << me->ability.getSkillGauge() << me->ability.getSpeed();
		me->userInfo->sendMsg(pk1);
		enemy->userInfo->sendMsg(pk2);

		// 순간이동
		me->crtList[0].position += me->crtList[0].direction * itemData->data[0];
		
		// 순간이동 패킷
		pk1.initBuffer();
		pk2.initBuffer();
		pk1 << SERVER_ITEMSKILL << true << getSlotIndex() << 19 << me->crtList[0].position;
		pk2 << SERVER_ITEMSKILL << false << getSlotIndex() << 19 << me->crtList[0].position;
		me->userInfo->sendMsg(pk1);
		enemy->userInfo->sendMsg(pk2);
	}

	matching.LeaveCS();
}
void Teleport::process() {
	matching.EnterCS();

	Player* me = matching.getPlayerOf(&user);
	Player* enemy = me->getEnemy();

	coolTime--;
	runTime++;

	// 스킬 시전 끝
	if (runTime == (int)(itemData->runTime * matching.getMatchingData().frame_rate)) {
		me->ability.runSkill = false;	// 시전 끝
		SendPacket p(30);
		SendPacket p2(30);
		p << GAME_PROTOCOL::SERVER_ITEMSKILL << true << getSlotIndex() << 2 << me->ability.getSpeed();
		p2 << GAME_PROTOCOL::SERVER_ITEMSKILL << false << getSlotIndex() << 2 << me->ability.getSpeed();
		me->userInfo->sendMsg(p);
		enemy->userInfo->sendMsg(p2);
	}

	matching.LeaveCS();
}
bool Teleport::isActived() {
	return (coolTime > 0);
}


Allurement::Allurement(User* u, Matching* m) :Item(u, m, ALLUREMENT)
{
}
void Allurement::recv() {
	int step;
	user >> step;

	if (step != 0)	return;

	SendPacket pk1(30), pk2(30);

	matching.EnterCS();
	Player* me = matching.getPlayerOf(&user);
	Player* enemy = me->getEnemy();

	if (coolTime == 0 && me->ability.getSkillGauge() >= itemData->gaugeCost &&
		me->canControl() && me->getState() == PLAYER_STATE::MOVE) {
		coolTime = itemData->coolTime * matching.getMatchingData().frame_rate;
		runTime = 0;

		me->ability.runSkill = true;
		me->ability.addSkillGauge(-itemData->gaugeCost);
		pk1 << GAME_PROTOCOL::SERVER_ITEMSKILL << true << getSlotIndex() << 1 << me->ability.getSkillGauge() << me->ability.getSpeed();
		pk2 << GAME_PROTOCOL::SERVER_ITEMSKILL << false << getSlotIndex() << 1 << me->ability.getSkillGauge() << me->ability.getSpeed();
		me->userInfo->sendMsg(pk1);
		enemy->userInfo->sendMsg(pk2);

		MagnetArrow* obj = new MagnetArrow(&user, &matching);
		obj->position = me->crtList[0].position + me->crtList[0].direction * matching.getMatchingData().crt_radius;
		obj->direction = me->crtList[0].direction;
		matching.addRuns(obj);

		pk1.initBuffer();
		pk1 << GAME_PROTOCOL::SERVER_GAMEOBJECT << obj->getSerial() << 3 << obj->getType() << obj->position << obj->direction;
		me->userInfo->sendMsg(pk1);
		enemy->userInfo->sendMsg(pk1);
	}

	matching.LeaveCS();
}
void Allurement::process() {
	matching.EnterCS();

	Player* me = matching.getPlayerOf(&user);
	Player* enemy = me->getEnemy();

	coolTime--;
	runTime++;

	// 스킬 시전 끝
	if (runTime == (int)(itemData->runTime * matching.getMatchingData().frame_rate)) {
		me->ability.runSkill = false;	// 시전 끝
		SendPacket p(30);
		SendPacket p2(30);
		p << GAME_PROTOCOL::SERVER_ITEMSKILL << true << getSlotIndex() << 2 << me->ability.getSpeed();
		p2 << GAME_PROTOCOL::SERVER_ITEMSKILL << false << getSlotIndex() << 2 << me->ability.getSpeed();
		me->userInfo->sendMsg(p);
		enemy->userInfo->sendMsg(p2);
	}

	matching.LeaveCS();
}
bool Allurement::isActived() {
	return (coolTime > 0);
}
