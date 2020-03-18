#include "User.h"
#include "GameObject.h"
#include "DataBase.h"
#include "StaticGameData.h"
#include "Skill.h"
#include "Item.h"
#include "Matching.h"

GameObject::GameObject(User* user, Matching* matching, ActiveData* data)
	: user(*user), matching(*matching), activeData(data)
{
	static int _serial = 0;
	serial = ++_serial;
}
GameObject::~GameObject()
{
}



Sword::Sword(User* u, Matching* m)
	:FieldObject(u, m, SWORD, &(db_skill[SKILL_TYPE::SHADOWSWORD])), actived(true), moveDistance(0)
{
}
float Sword::getRadius() {
	return activeData->data[D_HIT_RANGE];
}
void Sword::recv() {
	int step, crt_idx;
	user >> step >> crt_idx;
	if (step != 5) {
		return;
	}

	SendPacket *pk1 = nullptr, *pk2 = nullptr;
	SendPacket *pk3 = nullptr;

	matching.EnterCS();
	Player* me = matching.getPlayerOf(&user);
	Player* enemy = me->getEnemy();

	bool found;
	Character& c = enemy->getCharacter(crt_idx, found);

	if (found) {
		bool hit = matching.Action_hitTest_circle(c.position, matching.getMatchingData().crt_radius, position, getRadius());

		if (hit) {
			c.applyDamage(me->ability.getSkillDamage(activeData->data[D_DAMAGE]));
			me->ability.addSkillGauge(me->ability.getAttackRecovery());
			float hp = enemy->getCharacter(crt_idx, found).hp;

			actived = false;
			pk1 = new SendPacket(50);
			pk2 = new SendPacket(50);
			pk3 = new SendPacket(50);
			*pk1 << GAME_PROTOCOL::SERVER_GAMEOBJECT << serial << 6 << true << crt_idx << hp;
			*pk2 << GAME_PROTOCOL::SERVER_GAMEOBJECT << serial << 6 << false << crt_idx << hp;
			*pk3 << GAME_PROTOCOL::SERVER_GAMEOBJECT << serial << 4;

			me->sendPlayerDataToMe(PP_SKILLGAUGE);
		}
	}

	matching.LeaveCS();

	if (pk3 != nullptr) {
		me->userInfo->sendMsg(*pk3);
		enemy->userInfo->sendMsg(*pk3);
		delete pk3;
	}
	if (pk1 != nullptr) {
		me->userInfo->sendMsg(*pk1);
		delete pk1;
	}
	if (pk2 != nullptr) {
		enemy->userInfo->sendMsg(*pk2);
		delete pk2;
	}
}
void Sword::process() {

	SendPacket* packet = nullptr;

	matching.EnterCS();

	position += direction * (activeData->data[D_SPEED] / matching.getMatchingData().frame_rate);
	moveDistance += activeData->data[D_SPEED] / matching.getMatchingData().frame_rate;

	// 사거리 범위를 벗어나면
	if (moveDistance >= activeData->data[D_DISTANCE]) {
		actived = false;

		packet = new SendPacket(30);
		*packet << GAME_PROTOCOL::SERVER_GAMEOBJECT << serial << 4;
	}

	matching.LeaveCS();

	if (packet != nullptr) {
		user.sendMsg(*packet);
		matching.getEnemyOf(&user)->userInfo->sendMsg(*packet);
		delete packet;
	}
}
bool Sword::isActivited() {
	return actived;
}


Bubble::Bubble(User* u, Matching* m)
	:FieldObject(u, m, BUBBLE, &(db_skill[SKILL_TYPE::BUBBLETRAP])), actived(true), time(0), lock(0)
{
}
float Bubble::getRadius() {
	return activeData->data[D_HIT_RANGE];
}
void Bubble::recv() {
	if (actived == false) return;

	int step;
	user >> step;

	switch (step)
	{
	case 5://  적 공격
		attack();
		break;
	case 7:// 투사체 반사
		reflection();
		break;
	}
}
void Bubble::process() {
	SendPacket *pk1 = nullptr;
	SendPacket *pk2 = nullptr;
	SendPacket *pk3 = nullptr;

	matching.EnterCS();
	Player* me = matching.getPlayerOf(&user);
	Player* enemy = me->getEnemy();

	time++;

	// 방울과 적이 충돌 후
	// 적이 방울에 갇힌 상태
	if (lock > 0) {
		lock--;
		if (lock == 0) {// 물방울 탈출
			enemy->ability.lockedInBubble = false;

			pk2 = new SendPacket(30);
			pk3 = new SendPacket(30);
			*pk2 << GAME_PROTOCOL::SERVER_SKILL << false << serial << 9 << enemy->ability.getSpeed();
			*pk3 << GAME_PROTOCOL::SERVER_SKILL << true << serial << 9 << enemy->ability.getSpeed();
		}
		if (enemy->isStateOf(PLAYER_STATE::DEATH)) {// 죽었따
			lock = 0;
		}
	}

	// 방울 이동 상태
	if (actived) {
		position += direction * (activeData->data[D_SPEED] / matching.getMatchingData().frame_rate);

		// 시간 초과 시 제거
		if (time >= activeData->data[D_REMOVE_TIME] * matching.getMatchingData().frame_rate)
		{
			actived = false;

			pk1 = new SendPacket(30);
			*pk1 << GAME_PROTOCOL::SERVER_GAMEOBJECT << serial << 4;
		}
	}

	matching.LeaveCS();

	if (pk1 != nullptr) {
		me->userInfo->sendMsg(*pk1);
		enemy->userInfo->sendMsg(*pk1);
		delete pk1;
	}
	if (pk2 != nullptr) {
		me->userInfo->sendMsg(*pk2);
		delete pk2;
	}
	if (pk3 != nullptr) {
		enemy->userInfo->sendMsg(*pk3);
		delete pk3;
	}
}
bool Bubble::isActivited() {
	return actived || lock > 0;
}
void Bubble::attack()
{
	int crt_idx;
	user >> crt_idx;

	bool send;
	SendPacket pk1(80), pk2(80), pk3(30);

	matching.EnterCS();
	Player* me = matching.getPlayerOf(&user);
	Player* enemy = me->getEnemy();

	Character crt;
	bool found=false;
	crt = enemy->getCharacter(crt_idx, found);

	if (found) {
		bool hit = matching.Action_hitTest_circle(position, getRadius(), crt.position, matching.getMatchingData().crt_radius);
		if (hit) {
			send = true;
			enemy->ability.lockedInBubble = true;
			lock = activeData->data[D_ESCAPE_TIME] * matching.getMatchingData().frame_rate;
			actived = false;
			for (int i = enemy->crtCount-1; i >= 0; i--) {
				enemy->crtList[i].applyDamage(me->ability.getSkillDamage(activeData->data[D_DAMAGE]));
			}
			me->ability.addSkillGauge(me->ability.getAttackRecovery());
			pk1 << GAME_PROTOCOL::SERVER_GAMEOBJECT << serial << 6 << false << enemy->ability.getSpeed() << enemy->crtCount;
			pk2 << GAME_PROTOCOL::SERVER_GAMEOBJECT << serial << 6 << true << enemy->ability.getSpeed() << enemy->crtCount;
			for (int i = 0; i < enemy->crtCount; i++) {
				pk1 << enemy->crtList[i].index << enemy->crtList[i].hp;
				pk2 << enemy->crtList[i].index << enemy->crtList[i].hp;
			}

			pk3 << GAME_PROTOCOL::SERVER_GAMEOBJECT << serial << 4;

			me->sendPlayerDataToMe(PP_SKILLGAUGE);

		}
	}

	matching.LeaveCS();

	if (send) {
		me->userInfo->sendMsg(pk1);
		me->userInfo->sendMsg(pk3);
		enemy->userInfo->sendMsg(pk2);
		enemy->userInfo->sendMsg(pk3);
	}
}
void Bubble::reflection()
{
	int collision_type;
	int collider_idx;
	user >> collision_type;

	SendPacket pk1(40);
	bool send = false;

	matching.EnterCS();
	Player* me = matching.getPlayerOf(&user);
	Player* enemy = me->getEnemy();

	switch (collision_type)
	{
	case 0:	// 오브젝트
	{
		user >> collider_idx;
		if (collider_idx < matching.getMatchingData().obstacle.size()) {
			Obstacle& o = *(matching.getMatchingData().obstacle[collider_idx]);

			bool hit = matching.Action_reflection_circle(this, o.position, matching.getMatchingData().obstacle_radius);
			if (hit) {
				send = true;
				pk1 << GAME_PROTOCOL::SERVER_GAMEOBJECT << serial << 8 << position << direction;
			}
		}
	}
	break;
	case 1:	// 기믹
	{
		user >> collider_idx;
		Gimmick g = matching.getGimmick(collider_idx);

		if (g.index != MAP_OBJECT_TYPE::IDLE) {
			bool hit = matching.Action_reflection_circle(this, g.position, db_gimmick[g.index].radius);
			if (hit) {
				send = true;
				pk1 << GAME_PROTOCOL::SERVER_GAMEOBJECT << serial << 8 << position << direction;
			}
		}
	}
	break;
	case 2:	// 벽
		bool hit = matching.Action_reflection_Wall(this);
		if (hit) {
			send = true;
			pk1 << GAME_PROTOCOL::SERVER_GAMEOBJECT << serial << 8 << position << direction;
		}
		break;
	}

	matching.LeaveCS();

	if (send) {
		me->userInfo->sendMsg(pk1);
		enemy->userInfo->sendMsg(pk1);
	}
}



RabbitBomb::RabbitBomb(User* u, Matching* m, Rabbit* r) 
	:FieldObject(u, m, RABBIT_BOMB, &(db_item[ITEM_TYPE::RABBIT]))
{
	move = true;
	actived = true;
	distance = 0;
	runTime = 0;
	skill = r;
}
void RabbitBomb::recv() 
{
	int step;
	user >> step;

	if (step != 7) return;

	// 투사체 반사
	reflection();
}
void RabbitBomb::process() 
{
	matching.EnterCS();

	Player* me = matching.getPlayerOf(&user);
	Player* enemy = me->getEnemy();

	if (move) // 이동
	{
		distance += activeData->data[D_SPEED] / matching.getMatchingData().frame_rate;
		position += direction * activeData->data[D_SPEED];

		// 안착해야할 지 검사
		if (distance >= activeData->data[D_DISTANCE])
		{
			move = false;

			// 안착 패킷
			SendPacket sp(20);
			sp << GAME_PROTOCOL::SERVER_GAMEOBJECT << serial << 12 << position;

			me->userInfo->sendMsg(sp);
			enemy->userInfo->sendMsg(sp);
		}
	}
	else // 안착
	{
		runTime++;

		// 지속시간 검사
		if (runTime >= activeData->data[D_DURATION] * matching.getMatchingData().frame_rate)
		{
			actived = false;
			skill->bomb = nullptr;

			// 투사체 제거 패킷
			SendPacket sp(20);
			sp << GAME_PROTOCOL::SERVER_GAMEOBJECT << serial << 4;

			me->userInfo->sendMsg(sp);
			enemy->userInfo->sendMsg(sp);
		}
	}

	matching.LeaveCS();
}
bool RabbitBomb::isActivited() 
{
	return actived;
}
float RabbitBomb::getRadius() 
{
	return activeData->data[D_HIT_RANGE];
}
void RabbitBomb::reflection()
{
	int collision_type;
	int collider_idx;
	user >> collision_type;

	SendPacket pk(40);

	matching.EnterCS();
	Player* me = matching.getPlayerOf(&user);
	Player* enemy = me->getEnemy();

	switch (collision_type)
	{
	case 0:	// 오브젝트
	{
		user >> collider_idx;
		if (collider_idx < matching.getMatchingData().obstacle.size()) {
			Obstacle& o = *(matching.getMatchingData().obstacle[collider_idx]);

			bool hit = matching.Action_reflection_circle(this, o.position, matching.getMatchingData().obstacle_radius);
			if (hit) {
				pk << GAME_PROTOCOL::SERVER_GAMEOBJECT << serial << 8 << position << direction;
				me->userInfo->sendMsg(pk);
				enemy->userInfo->sendMsg(pk);
			}
		}
	}
	break;
	case 1:	// 기믹
	{
		user >> collider_idx;
		Gimmick g = matching.getGimmick(collider_idx);

		if (g.index != MAP_OBJECT_TYPE::IDLE) {
			bool hit = matching.Action_reflection_circle(this, g.position, db_gimmick[g.index].radius);
			if (hit) {
				pk << GAME_PROTOCOL::SERVER_GAMEOBJECT << serial << 8 << position << direction;
				me->userInfo->sendMsg(pk);
				enemy->userInfo->sendMsg(pk);
			}
		}
	}
	break;
	case 2:	// 벽
		bool hit = matching.Action_reflection_Wall(this);
		if (hit) {
			pk << GAME_PROTOCOL::SERVER_GAMEOBJECT << serial << 8 << position << direction;
			me->userInfo->sendMsg(pk);
			enemy->userInfo->sendMsg(pk);
		}
		break;
	}

	matching.LeaveCS();
}
void RabbitBomb::explosion()
{
	matching.EnterCS();
	Player* me = matching.getPlayerOf(&user);

	//for (int i = 0; i < 2; i++) 
	{// 적만 피격
		//Player* p = matching.getPlayer(i);
		Player* p = matching.getEnemyOf(&user);
		
		bool someHit = false;
		// 폭발 범위에 있는 캐릭터 데미지
		for (int j = p->crtCount-1; j >= 0; j--)
		{
			bool hit = matching.Action_hitTest_circle(
				position, activeData->data[D_BOMB_RANGE], 
				p->crtList[j].position, matching.getMatchingData().crt_radius);
			if (hit) {
				someHit = true;
				p->crtList[j].applyDamage(me->ability.getSkillDamage(activeData->data[D_DAMAGE]));
			}
		}
		// 충돌 패킷 send
		if (someHit) {
			me->ability.addSkillGauge(me->ability.getAttackRecovery());

			SendPacket sp(60), sp2(60);
			sp << SERVER_GAMEOBJECT << serial << 6 << true << p->ability.getSpeed() << p->crtCount;
			sp2 << SERVER_GAMEOBJECT << serial << 6 << false << p->ability.getSpeed() << p->crtCount;
			for (int j = 0; j < p->crtCount; j++)
			{
				sp << p->crtList[j].index << p->crtList[j].hp;
				sp2 << p->crtList[j].index << p->crtList[j].hp;
			}
			p->userInfo->sendMsg(sp);
			p->getEnemy()->userInfo->sendMsg(sp2);

			me->sendPlayerDataToMe(PP_SKILLGAUGE);

		}
		// 투사체 제거 패킷
	}

	SendPacket sp(20);
	sp << GAME_PROTOCOL::SERVER_GAMEOBJECT << serial << 4;
	matching.getPlayer1()->userInfo->sendMsg(sp);
	matching.getPlayer2()->userInfo->sendMsg(sp);

	actived = false;
	skill->bomb = nullptr;

	matching.LeaveCS();
}


Arrow::Arrow(User* u, Matching* m) 
	:FieldObject(u, m, ARROW, &(db_item[ITEM_TYPE::ARROWSNIPE]))
{
	time = 0;
	actived = true;
}
void Arrow::recv() 
{
	int step, crt_idx;
	user >> step;

	if (step != 5) return;

	user >> crt_idx;
	
	matching.EnterCS();
	Player* me = matching.getPlayerOf(&user);
	Player* enemy = me->getEnemy();

	bool found;
	Character& c = enemy->getCharacter(crt_idx, found);
	if (found) {
		bool hit = matching.Action_hitTest_circle(
			position, activeData->data[D_HIT_RANGE],
			c.position, matching.getMatchingData().crt_radius);
		if (hit) {
			c.applyDamage(me->ability.getSkillDamage(activeData->data[D_DAMAGE]));
			me->ability.addSkillGauge(me->ability.getAttackRecovery());

			actived = false;
			enemy->ability.stun = activeData->data[D_STUN_TIME] * matching.getMatchingData().frame_rate;

			SendPacket sp(60), sp2(60);
			sp << SERVER_GAMEOBJECT << serial << 6 << false << enemy->ability.getSpeed() << enemy->crtCount;
			sp2 << SERVER_GAMEOBJECT << serial << 6 << true << enemy->ability.getSpeed() << enemy->crtCount;
			for (int j = 0; j < enemy->crtCount; j++)
			{
				sp << enemy->crtList[j].index << enemy->crtList[j].hp;
				sp2 << enemy->crtList[j].index << enemy->crtList[j].hp;
			}
			me->userInfo->sendMsg(sp);
			enemy->getEnemy()->userInfo->sendMsg(sp2);

			sp.initBuffer();
			sp << GAME_PROTOCOL::SERVER_GAMEOBJECT << serial << 4;

			me->userInfo->sendMsg(sp);
			enemy->userInfo->sendMsg(sp);

			me->sendPlayerDataToMe(PP_SKILLGAUGE);
			enemy->sendPlayerDataToMe(PP_STUN);
			enemy->sendPlayerDataToEnemy(PP_STUN);
		}
	}

	matching.LeaveCS();
}
void Arrow::process() 
{
	matching.EnterCS();
	Player* me = matching.getPlayerOf(&user);
	Player* enemy = me->getEnemy();

	time++;
	position += direction * (activeData->data[D_SPEED] / matching.getMatchingData().frame_rate);

	if (time >= activeData->data[D_DESTROY_TIME] * matching.getMatchingData().frame_rate)
	{
		actived = false;

		// 투사체 제거 패킷
		SendPacket sp(20);
		sp << GAME_PROTOCOL::SERVER_GAMEOBJECT << serial << 4;

		me->userInfo->sendMsg(sp);
		enemy->userInfo->sendMsg(sp);
	}

	matching.LeaveCS();
}
bool Arrow::isActivited() 
{
	return actived;
}
float Arrow::getRadius() 
{
	return activeData->data[D_HIT_RANGE];
}



BlackHole::BlackHole(User* u, Matching* m) 
	:FieldObject(u, m, BLACK_HOLE, &(db_item[ITEM_TYPE::NIGHTHOLLOW]))
{
	actived = true;
	time = 0;
}
void BlackHole::recv()
{
}
void BlackHole::process() 
{
	matching.EnterCS();
	Player* me = matching.getPlayerOf(&user);
	Player* enemy = me->getEnemy();

	time++;
	if (time >= activeData->data[D_DESTROY_TIME] * matching.getMatchingData().frame_rate)
	{
		actived = false;

		// 투사체 제거 패킷
		SendPacket sp(20);
		sp << GAME_PROTOCOL::SERVER_GAMEOBJECT << serial << 4;

		me->userInfo->sendMsg(sp);
		enemy->userInfo->sendMsg(sp);
	}
	else
	{
		float distance = (position - enemy->crtList[0].position).size();

		if (distance < 2) // 블랙홀 중심과 충돌
		{
			for (int i = enemy->crtCount - 1; i >= 0; i--)
			{
				enemy->crtList[i].applyDamage(me->ability.getSkillDamage(activeData->data[D_DAMAGE]));
			}
			me->ability.addSkillGauge(me->ability.getAttackRecovery());

			SendPacket sp(60), sp2(60);
			sp << SERVER_GAMEOBJECT << serial << 6 << false << enemy->ability.getSpeed() << enemy->crtCount;
			sp2 << SERVER_GAMEOBJECT << serial << 6 << true << enemy->ability.getSpeed() << enemy->crtCount;
			for (int i = 0; i < enemy->crtCount; i++) {
				sp << enemy->crtList[i].index << enemy->crtList[i].hp;
				sp2 << enemy->crtList[i].index << enemy->crtList[i].hp;
			}
			me->userInfo->sendMsg(sp);
			enemy->userInfo->sendMsg(sp2);


			actived = false;

			// 투사체 제거 패킷
			sp.initBuffer();
			sp << GAME_PROTOCOL::SERVER_GAMEOBJECT << serial << 4;

			me->userInfo->sendMsg(sp);
			enemy->userInfo->sendMsg(sp);
			me->sendPlayerDataToMe(PP_SKILLGAUGE);
		}
		else if(distance < activeData->data[D_HIT_RANGE]) // 자석 끌어당김
		{
			float percent = activeData->data[D_HIT_RANGE] - distance;
			percent /= activeData->data[D_HIT_RANGE];
			float power = activeData->data[D_MAGNET_POWER] * percent / matching.getMatchingData().frame_rate;
			Vector2 direction = (position - enemy->crtList[0].position).normalization();

			enemy->crtList[0].position += direction * power;
			enemy->currentDistance += power;
		}
	}

	matching.LeaveCS();
}
bool BlackHole::isActivited() 
{
	return actived;
}



MagnetArrow::MagnetArrow(User* u, Matching* m) 
	:FieldObject(u, m, MAGNET_ARROW, &(db_item[ITEM_TYPE::ALLUREMENT]))
{
	actived = true;
	distance = 0;
	allurementTime = 0;
}
void MagnetArrow::recv() 
{
	int step, crt_idx;
	user >> step;

	if (step != 5) return;

	user >> crt_idx;

	matching.EnterCS();
	Player* me = matching.getPlayerOf(&user);
	Player* enemy = me->getEnemy();

	bool found;
	Character& c = enemy->getCharacter(crt_idx, found);
	if (found) {
		bool hit = matching.Action_hitTest_circle(
			position, activeData->data[D_HIT_RANGE],
			c.position, matching.getMatchingData().crt_radius);
		if (hit) {
			c.applyDamage(me->ability.getSkillDamage(activeData->data[D_DAMAGE]));
			me->ability.addSkillGauge(me->ability.getAttackRecovery());

			actived = false;
			allurementTime = activeData->data[D_MAGNET_TIME] * matching.getMatchingData().frame_rate;
			enemy->ability.ignoreControl = allurementTime;
			enemy->ability.allurement = true;

			SendPacket sp(60), sp2(60);
			sp << SERVER_GAMEOBJECT << serial << 6 << false << enemy->ability.getSpeed() << enemy->crtCount;
			sp2 << SERVER_GAMEOBJECT << serial << 6 << true << enemy->ability.getSpeed() << enemy->crtCount;
			for (int j = 0; j < enemy->crtCount; j++)
			{
				sp << enemy->crtList[j].index << enemy->crtList[j].hp;
				sp2 << enemy->crtList[j].index << enemy->crtList[j].hp;
			}
			me->userInfo->sendMsg(sp);
			enemy->getEnemy()->userInfo->sendMsg(sp2);

			sp.initBuffer();
			sp << GAME_PROTOCOL::SERVER_GAMEOBJECT << serial << 4;

			me->userInfo->sendMsg(sp);
			enemy->userInfo->sendMsg(sp);

			me->sendPlayerDataToMe(PP_SKILLGAUGE);
			enemy->sendPlayerDataToMe(PP_IGNORE_CONTROL);
		}
	}
	bool hit = matching.Action_hitTest_circle(enemy, position, activeData->data[D_HIT_RANGE]);


	matching.LeaveCS();
}
void MagnetArrow::process() 
{
	matching.EnterCS();
	Player* me = matching.getPlayerOf(&user);
	Player* enemy = me->getEnemy();

	position += direction * (activeData->data[D_SPEED] / matching.getMatchingData().frame_rate);
	distance += activeData->data[D_SPEED] / matching.getMatchingData().frame_rate;

	// 사거리 범위를 벗어나면
	if (distance >= activeData->data[D_DISTANCE] && actived) {
		actived = false;

		SendPacket sp(30);
		sp << GAME_PROTOCOL::SERVER_GAMEOBJECT << serial << 4;
		me->userInfo->sendMsg(sp);
		enemy->userInfo->sendMsg(sp);
	}
	if (allurementTime > 0) {
		allurementTime--;
		enemy->joystick = -999;
		enemy->crtList[0].direction = (me->crtList[0].position - enemy->crtList[0].position).normalization();

		if (allurementTime == 0) {
			enemy->ability.allurement = false;
			enemy->sendPlayerDataToMe(PP_SPEED);
			enemy->sendPlayerDataToEnemy(PP_SPEED);
		}
	}

	matching.LeaveCS();
}
bool MagnetArrow::isActivited() 
{
	return actived || (allurementTime > 0);
}
float MagnetArrow::getRadius() 
{
	return activeData->data[D_HIT_RANGE];
}