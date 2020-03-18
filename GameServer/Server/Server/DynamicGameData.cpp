#include "DynamicGameData.h"
#include "Global.h"
#include "Matching.h"
#include "DataBase.h"
#include "DebugClientManager.h"
#include "TCPClient.h"
#include <map>

#define	ERROR -1

enum CRT_IDX {
	LEADER = 0,
	SUB1,
	SUB2,
	SUB3
};

/* Gimmick Section */

// Gimmick
void Gimmick::packing(SendPacket& packet)
{
	packet << index << position << db_gimmick[index].radius;
}
Gimmick* Gimmick::createGimmick(MAP_OBJECT_TYPE type, Gimmick* copyTarget)
{
#define copy(_class) (copyTarget == nullptr ? new _class : new _class(*(_class*)copyTarget))

	Gimmick* g;
	switch (type) {
	case MAP_OBJECT_TYPE::GIMMICK_BUSH:
		g = copy(Bush);
		break;
	case MAP_OBJECT_TYPE::GIMMICK_MANDRA:
		g = copy(Mandora);
		break;
	case MAP_OBJECT_TYPE::GIMMICK_MARSH:
		g = copy(Marsh);
		break;
	case MAP_OBJECT_TYPE::GIMMICK_CASTLE_BLUE:
	case MAP_OBJECT_TYPE::GIMMICK_CASTLE_RED:
		g = copy(Castle);
		break;
	case MAP_OBJECT_TYPE::GIMMICK_TEMPLE:
		g = copy(Temple);
		break;
	case MAP_OBJECT_TYPE::GIMMICK_GEM:
		g = copy(Gem);
		break;
	case MAP_OBJECT_TYPE::GIMMICK_BLESSINGTREE:
		g = copy(BlessingTree);
		break;
	case MAP_OBJECT_TYPE::GIMMICK_CAT:
		g = copy(Cat);
		break;
	default:
		g = copy(Gimmick);
		break;
	}
	g->index = type;
	return g;
}
// Temple
bool Temple::run()
{
	char msg[100];
	matching->EnterCS();

	if (firstPlayer == nullptr) {
		casting = false;
	}
	bool sendTempleInfo = false;
	if (cooltime > 0) {
		cooltime--;
		if (cooltime%matching->getMatchingData().frame_rate == 0) {
			sendTempleInfo = true;
		}
	}
	Player* successPlayer = nullptr;
	if (casting)
	{
		if (firstPlayer == matching->getPlayer1())
		{
			gauge += 0.5f;
			if (gauge >= maxGauge) {
				successPlayer = firstPlayer;
			}
		}
		else if (firstPlayer == matching->getPlayer2())
		{
			gauge -= 0.5f;
			if (gauge <= -maxGauge) {
				successPlayer = firstPlayer;
			}
		}

		// 캐스팅 완료
		if (successPlayer != nullptr) {
			gauge = 0;
			casting = false;
			cooltime = db_gimmick[index].data[0] * matching->getMatchingData().frame_rate;
			firstPlayer = nullptr;
			sprintf(msg, "템플 게이지 100");
			successPlayer->score += db_gimmick[index].data[1];
			switch (gemType) {
			case A:
				if (successPlayer->ability.templeDamage < db_gimmick[index].data[2] * 3)
					successPlayer->ability.templeDamage += db_gimmick[index].data[2] * 3;
				gemType = B; break;
			case B:
				if (successPlayer->ability.templeHp < db_gimmick[index].data[3] * 3)
					successPlayer->ability.templeHp += db_gimmick[index].data[3] * 3;
				gemType = C; break;
			case C:
				if (successPlayer->ability.templeRecovery < db_gimmick[index].data[4] * 3)
					successPlayer->ability.templeRecovery += db_gimmick[index].data[4] * 3;
				gemType = A; break;
			}
			successPlayer->sendPlayerDataToMe(PP_SCORE | PP_MAXHP_ALL | PP_RECOVERY_HP);
			successPlayer->sendPlayerDataToEnemy(PP_SCORE | PP_MAXHP_ALL | PP_RECOVERY_HP);
		}
		else {// 캐스팅 중
			Player* player = firstPlayer;
			player->ability.addStamina(-player->ability.getStaminaDecrementOfCasting() / player->matching->getMatchingData().frame_rate);
			if (player->ability.getStamina() <= 0) {
				player->cancelCasting(false);
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
	}

	matching->LeaveCS();
	if (sendTempleInfo) {
		DebugClientManager::GetInstance()->sendChangeTemple(matching);
	}
	if (successPlayer != nullptr)
	{
		User& player = *(successPlayer->userInfo);
		User& enemy = *(matching->getEnemyOf(&player)->userInfo);
		SendPacket pk1(10), pk2(10), pk3(30), pk4(30);

		pk1 << GAME_PROTOCOL::SERVER_TEMPLE_SUCCESS << true << gauge;
		pk2 << GAME_PROTOCOL::SERVER_TEMPLE_SUCCESS << false << gauge;

		successPlayer->ability.speedPer = 1;
		successPlayer->setState(PLAYER_STATE::MOVE);
		pk3 << GAME_PROTOCOL::SERVER_TEMPLE_CASTING << true << false << successPlayer->ability.getSpeed() << gauge << false;
		pk4 << GAME_PROTOCOL::SERVER_TEMPLE_CASTING << false << false << successPlayer->ability.getSpeed() << gauge << false;

		player.sendMsg(pk1);
		enemy.sendMsg(pk2);
		player.sendMsg(pk3);
		enemy.sendMsg(pk4);

		DebugClientManager::GetInstance()->sendChangeTemple(matching);
		DebugClientManager::GetInstance()->sendEvent(matching, successPlayer, DEBUG_PROTOCOL::SERVER_EVENT_TEMPLE, msg);
	}

	return true;
}
void Temple::packing(SendPacket& packet)
{
	packet << index << position << db_gimmick[index].radius;
	packet << maxGauge << gauge << casting << cooltime;
	if (firstPlayer == nullptr)
		packet << -1;
	else if (firstPlayer == matching->getPlayer1())
		packet << 0;
	else if (firstPlayer == matching->getPlayer2())
		packet << 1;
}
bool Temple::isEnter(Player* p)
{
	return matching->Action_hitTest_circle(p, position, db_gimmick[index].radius);
}
bool Temple::enter(Player& p, bool _bool)
{
	if (_bool) {
		bool hit = isEnter(&p);
		bool canEnter = (firstPlayer == nullptr || firstPlayer == &p);
		bool active = (cooltime == 0);
		bool live = p.getState() != PLAYER_STATE::DEATH;
		bool result = (hit && canEnter && active && live && (allIntoTemple == false));
		if (result)
			firstPlayer = &p;
		else if (firstPlayer == &p) {
			firstPlayer = nullptr;
			allIntoTemple = false;
		}
		return result;
	}
	else {
		if (firstPlayer == &p) {
			if (casting) {
				return false;
			}
			firstPlayer = nullptr;

			bool hit = isEnter(p.getEnemy());
			bool active = (cooltime == 0);
			bool live = p.getEnemy()->getState() != PLAYER_STATE::DEATH;
			bool result = hit && active && live;
			if (result) {
				firstPlayer = p.getEnemy();
			}
		}
		else if (firstPlayer == nullptr) {

			bool hit = isEnter(p.getEnemy());
			bool active = (cooltime == 0);
			bool live = p.getEnemy()->getState() != PLAYER_STATE::DEATH;
			bool result = hit && active && live;
			if (result) {
				firstPlayer = p.getEnemy();
			}
		}
		if (&p == matching->getPlayer1() && gauge > 0)
			gauge = 0;
		if (&p == matching->getPlayer2() && gauge < 0)
			gauge = 0;
		allIntoTemple = false;
		return true;
	}
}
bool Temple::isCasting(Player* p)
{
	return (casting && firstPlayer == p);
}
bool Temple::isCanCasting(Player* p)
{
	return (cooltime == 0 &&
		firstPlayer == p &&
		allIntoTemple == false &&
		p->getState() != PLAYER_STATE::DEATH);
}
bool Temple::tryToCasting(Player& p, Player& enemy, bool _bool, bool autoLogSend)
{
	if (_bool) {
		bool canStart = (
			cooltime == 0 &&
			firstPlayer == &p &&
			allIntoTemple == false &&
			p.getState() != PLAYER_STATE::DEATH);

		if (canStart)
		{
			casting = true;
			p.ability.speedPer = 0;
			p.setState(PLAYER_STATE::MOVE, autoLogSend);
		}
		return canStart;
	}
	else {
		if (casting && firstPlayer == &p)
		{
			casting = false;
			allIntoTemple = false;
			if (p.getState() != PLAYER_STATE::DEATH) {
				p.ability.speedPer = 1;
				p.setState(PLAYER_STATE::MOVE, autoLogSend);
			}
			else {
				firstPlayer = nullptr;
				gauge = 0;
			}
			if (isEnter(&p) && isEnter(&enemy) &&
				p.getState() != PLAYER_STATE::DEATH && enemy.getState() != PLAYER_STATE::DEATH)
			{
				allIntoTemple = true;
				firstPlayer = nullptr;
			}
		}

		return false;
	}
}
// Mandora
bool Mandora::run() {
	matching->EnterCS();

	// 스스로의 idx를 받아옴
	int field_idx = matching->getIndexOf(*this);
	// 만도라가 matching->runGimmicks에 잔류해도 되는지를 체크
	bool continueRun = true;
	// 만도라가 재생성 되는지를 체크
	bool _respawn = false;
	// 
	bool _explosion[2] = { false, false };

	switch (stat) {
	case STAT::NORMAL:
		break;
	case STAT::ACTIVITY:
		timer++;
		// 폭말 해야 할 타이밍이라면
		if (timer >= matching->getMatchingData().frame_rate * db_gimmick[index].data[1]) { // 폭발 카운팅
			stat = STAT::DIE;
			// 타이머 초기화
			timer = 0;
			_explosion[0] = explosion(matching->getPlayer1());
			_explosion[1] = explosion(matching->getPlayer2());
		}
		break;
	case STAT::DIE:
		timer++;
		// 재생성 해야 될 타이밍이라면
		if (timer >= matching->getMatchingData().frame_rate * db_gimmick[index].data[2]) { // 리스폰 카운팅
			stat = STAT::NORMAL;
			timer = 0;
			_respawn = true;
			continueRun = false;
		}
		break;
	}
	matching->LeaveCS();

	// 플레이어 캐릭터들의 폭발 시 범위 포함을 체크
	for (int i = 0; i < 2; i++) {
		Player& p = *(matching->getPlayer(i));
		// 폭발에 휩쓸렸다면
		if (_explosion[i]) {
			SendPacket pk1(70), pk2(70);
			pk1 << GAME_PROTOCOL::SERVER_COLLISION_GIMMICK << true << index << field_idx << 1 << p.crtCount;
			for (int j = 0; j < p.crtCount; j++)
				pk1 << p.crtList[j].index << p.crtList[j].hp;
			pk2 << GAME_PROTOCOL::SERVER_COLLISION_GIMMICK << false << index << field_idx << 1 << p.crtCount;
			for (int j = 0; j < p.crtCount; j++)
				pk2 << p.crtList[j].index << p.crtList[j].hp;

			p.userInfo->sendMsg(pk1);
			matching->getEnemyOf(p.userInfo)->userInfo->sendMsg(pk2);
		}
		// 리스폰 해도 될 타이밍이 왔다면
		if (_respawn) {
			SendPacket pk(30);
			pk << GAME_PROTOCOL::SERVER_COLLISION_GIMMICK << true << index << field_idx << 2;
			p.userInfo->sendMsg(pk);
		}
	}

	return continueRun;
}
bool Mandora::explosion(Player* p)
{
	bool hit = false;
	for (int i = p->crtCount - 1; i >= 0; i--) {
		if (matching->Action_hitTest_circle(position, db_gimmick[index].data[0], p->crtList[i].position, matching->getMatchingData().crt_radius)) {
			//if (hitTest(p->crtList[i].position)) {
			p->applyDamage(i, db_gimmick[index].data[3]);
			hit = true;
		}
	}
	return hit;
}
void Mandora::recv(User& user) {
	matching->EnterCS();

	int field_idx = matching->getIndexOf(*this);

	if (field_idx == ERROR) {
		matching->LeaveCS();
		return;
	}

	Player& p_me = *(matching->getPlayerOf(&user));
	Player& p_enemy = *(matching->getEnemyOf(&user));
	User& enemy = *(p_enemy.userInfo);

	SendPacket collision1(40), collision2(40);

	bool send_hit = false;
	bool hit = matching->Action_hitTest_circle(&p_me, position, db_gimmick[index].radius);
	// 상태가 죽어있는 상태가 아니면서, 충돌 하였다면
	if (stat != STAT::DIE && hit) {
		// 충돌을 보내야 한다
		send_hit = true;
		matching->Action_reflection_circle(&p_me, position, db_gimmick[index].radius);
		// 충돌하였다는 정보를 패킹
		// index => 기믹의 종류
		// field_idx => 맵의 기믹들 중 몇번째 기믹인가를 의미
		collision1 << GAME_PROTOCOL::SERVER_COLLISION_GIMMICK << true << index << field_idx << 0;
		collision2 << GAME_PROTOCOL::SERVER_COLLISION_GIMMICK << false << index << field_idx << 0;
		// 충돌 하였을 경우 튕겨져야 한다.
		collision1 << p_me.crtList[0].direction << p_me.crtList[0].position;
		collision2 << p_me.crtList[0].direction << p_me.crtList[0].position;

		// 활성화
		if (stat == STAT::NORMAL) {
			// 활성화 상태로 바꿔주기
			stat = STAT::ACTIVITY;
			// 타이머 스타트
			timer = 0;
			// 자신을 포함하고 있는 matching의 runGimmicks에 스스로를 추가
			// matching의 process 때 마다 이 기믹의 run()을 실행 할 것이다
			matching->addRuns(this);
		}
	}

	matching->LeaveCS();

	if (send_hit) {
		user.sendMsg(collision1);
		enemy.sendMsg(collision2);
	}
}
// Marsh
void Marsh::recv(User& user)
{
	matching->EnterCS();

	Player& p_me = *(matching->getPlayerOf(&user));

	Rect rt = Rect(position, position);
	Vector2 delta = Vector2(db_gimmick[index].radius, db_gimmick[index].radius);
	rt.leftBottom -= delta;
	rt.rightTop += delta;
	bool hit = matching->Action_hitTest_box(p_me.crtList[0].position, matching->getMatchingData().crt_radius, rt);

	SendPacket pk1(30), pk2(30);
	if (hit)
	{
		for (int i = 0; i < player.size(); i++) {
			if (player[i] == &p_me)
				break;
			// 끝까지 순회하고도 못찾았을 때
			if (i == player.size() - 1)
				player.push_back(&p_me);
		}
		if (player.size() == 0)
			player.push_back(&p_me);

		p_me.ability.marshSpeed = db_gimmick[index].data[0];
		matching->addRuns(this);

		int field_idx = matching->getIndexOf(*this);
		pk1 << GAME_PROTOCOL::SERVER_COLLISION_GIMMICK;
		pk1 << true << index << field_idx << 0 << p_me.ability.getSpeed();
		pk2 << GAME_PROTOCOL::SERVER_COLLISION_GIMMICK;
		pk2 << false << index << field_idx << 0 << p_me.ability.getSpeed();
	}

	matching->LeaveCS();
	if (hit) {
		user.sendMsg(pk1);
		matching->getEnemyOf(&user)->userInfo->sendMsg(pk2);
	}
}
bool Marsh::run()
{
	matching->EnterCS();

	Rect rt = Rect(position, position);
	Vector2 delta = Vector2(db_gimmick[index].radius, db_gimmick[index].radius);
	rt.leftBottom -= delta;
	rt.rightTop += delta;

	int count = player.size();
	std::map<User*, SendPacket*> send;
	for (int i = 0; i < count; i++) {
		Player& p_me = *(player[i]);

		bool hit = matching->Action_hitTest_box(p_me.crtList[0].position, matching->getMatchingData().crt_radius, rt);
		if (hit == false) {
			p_me.ability.marshSpeed = 0;
			player.erase(player.begin() + i);
			i--; count--;

			int field_idx = matching->getIndexOf(*this);
			SendPacket& pk1 = *(new SendPacket(30));
			SendPacket& pk2 = *(new SendPacket(30));
			pk1 << GAME_PROTOCOL::SERVER_COLLISION_GIMMICK;
			pk1 << true << index << field_idx << 0 << p_me.ability.getSpeed();
			pk2 << GAME_PROTOCOL::SERVER_COLLISION_GIMMICK;
			pk2 << false << index << field_idx << 0 << p_me.ability.getSpeed();
			send[p_me.userInfo] = &pk1;
			send[p_me.getEnemy()->userInfo] = &pk2;
		}
	}

	matching->LeaveCS();

	for (auto dt : send) {
		dt.first->sendMsg(*dt.second);
		delete dt.second;
	}

	if (player.size() == 0)
		return false;
	return true;
}
// Castle
void Castle::recv(User& user)
{
	matching->EnterCS();

	Player& p_me = *(matching->getPlayerOf(&user));

	Rect rt = Rect(position, position);
	Vector2 delta = Vector2(db_gimmick[index].radius, db_gimmick[index].radius);
	rt.leftBottom -= delta;
	rt.rightTop += delta;
	//bool hit = matching->Action_hitTest_box(p_me.crtList[0].position, db_matching.crt_radius, rt);
	bool hit = matching->Action_hitTest_circle(&p_me, position, db_gimmick[index].radius);

	SendPacket pk1(50), pk2(50);

	if (hit) {
		int field_idx = matching->getIndexOf(*this);
		//matching->Action_reflection_box(&p_me, db_matching.crt_radius, rt);
		matching->Action_reflection_circle(&p_me, position, db_gimmick[index].radius);

		pk1 << GAME_PROTOCOL::SERVER_COLLISION_GIMMICK;
		pk1 << true << index << field_idx << 0 << p_me.crtList[0].direction << p_me.crtList[0].position;
		pk2 << GAME_PROTOCOL::SERVER_COLLISION_GIMMICK;
		pk2 << false << index << field_idx << 0 << p_me.crtList[0].direction << p_me.crtList[0].position;
	}

	matching->LeaveCS();
	if (hit) {
		p_me.userInfo->sendMsg(pk1);
		p_me.getEnemy()->userInfo->sendMsg(pk2);
	}
}
// Gem
std::map<int, unsigned int> Gem::startFrame;
void Gem::recv(User& user)
{
	matching->EnterCS();

	if (activity == false) {
		matching->LeaveCS();
		return;
	}

	Player& p_me = *(matching->getPlayerOf(&user));
	bool hit = matching->Action_hitTest_circle(&p_me, position, db_gimmick[index].radius);
	SendPacket pk1(30), pk2(30);
	if (hit)
	{
		acquire(&p_me);

		p_me.sendPlayerDataToMe(PP_SKILLGAUGE);

		int field_idx = matching->getIndexOf(*this);
		pk1 << GAME_PROTOCOL::SERVER_COLLISION_GIMMICK;
		pk1 << true << index << field_idx << 0 << p_me.score;
		pk2 << GAME_PROTOCOL::SERVER_COLLISION_GIMMICK;
		pk2 << false << index << field_idx << 0 << p_me.score;
	}

	matching->LeaveCS();

	if (hit) {
		p_me.userInfo->sendMsg(pk1);
		p_me.getEnemy()->userInfo->sendMsg(pk2);
	}
}
bool Gem::acquire(Player* p)
{
	if (!activity) return false;
	activity = false;
	p->score += db_gimmick[index].data[1];
	p->ability.addSkillGauge(db_gimmick[index].data[2]);

	int n = Gem::startFrame[matching->getSerial()];
	if (Gem::startFrame[matching->getSerial()] == 0) {
		Gem::startFrame[matching->getSerial()] = matching->getCurrentFrame();
		matching->addRuns(this);
	}
	return true;
}
bool Gem::run()
{
	matching->EnterCS();

	bool continou = true;

	SendPacket *pk = nullptr;
	bool timeover = matching->getCurrentFrame() - Gem::startFrame[matching->getSerial()] >= db_gimmick[index].data[0] * matching->getMatchingData().frame_rate;
	if (timeover) {
		int count = matching->getGimmicksCount();
		for (int i = 0; i < count; i++) {
			Gimmick& g = matching->getGimmick(i);
			if (g.index == index) {
				((Gem*)&g)->activity = true;
			}
		}
		Gem::startFrame[matching->getSerial()] = 0;
		continou = false;

		int field_idx = matching->getIndexOf(*this);
		pk = new SendPacket(30);
		*pk << GAME_PROTOCOL::SERVER_COLLISION_GIMMICK;
		*pk << true << index << field_idx << 1;
	}

	matching->LeaveCS();

	if (timeover) {
		matching->getPlayer1()->userInfo->sendMsg(*pk);
		matching->getPlayer2()->userInfo->sendMsg(*pk);
		delete pk;
	}

	return continou;
}
// BlessingTree
void BlessingTree::recv(User& user)
{
	matching->EnterCS();

	Player* p_me = matching->getPlayerOf(&user);

	SendPacket pk1(80), pk2(80);

	int field_idx = matching->getIndexOf(*this);
	bool hit = matching->Action_hitTest_circle(p_me, position, db_gimmick[index].radius);
	bool send = false;
	if (hit && activity) {
		if (p_me->isStateOf(PLAYER_STATE::MOVE)) {
			send = true;
			type = (++type) % 3;
			matching->Action_reflection_circle(p_me, position, db_gimmick[index].radius);
			pk1 << GAME_PROTOCOL::SERVER_COLLISION_GIMMICK << true << index << field_idx << 0;
			pk1 << p_me->crtList[0].direction << p_me->crtList[0].position << type;
			pk2 << GAME_PROTOCOL::SERVER_COLLISION_GIMMICK << false << index << field_idx << 0;
			pk2 << p_me->crtList[0].direction << p_me->crtList[0].position << type;
		}
		else if (p_me->isStateOf(PLAYER_STATE::ATTACK)) {
			send = true;
			matching->Action_reflection_circle(p_me, position, db_gimmick[index].radius);
			pk1 << GAME_PROTOCOL::SERVER_COLLISION_GIMMICK << true << index << field_idx << 1;
			pk1 << p_me->crtList[0].direction << p_me->crtList[0].position << type;
			pk2 << GAME_PROTOCOL::SERVER_COLLISION_GIMMICK << false << index << field_idx << 1;
			pk2 << p_me->crtList[0].direction << p_me->crtList[0].position << type;
			switch (type) {
			case 0://체력
				pk1 << p_me->crtCount;
				pk2 << p_me->crtCount;
				for (int i = 0; i < p_me->crtCount; i++) {
					p_me->crtList[i].addHP(db_gimmick[index].data[type]);
					pk1 << p_me->crtList[i].index << p_me->crtList[i].hp;
					pk2 << p_me->crtList[i].index << p_me->crtList[i].hp;
				}
				break;
			case 1://스킬게이지
				p_me->ability.addSkillGauge(db_gimmick[index].data[type]);
				pk1 << p_me->ability.getSkillGauge();
				pk2 << p_me->ability.getSkillGauge();
				break;
			case 2://스테미나
				p_me->ability.addStamina(db_gimmick[index].data[type]);
				pk1 << p_me->ability.getStamina();
				pk2 << p_me->ability.getStamina();
				break;
			}
			deathCount = 0;
			activity = false;
			matching->addRuns(this);
		}
	}

	matching->LeaveCS();

	if (send) {
		p_me->userInfo->sendMsg(pk1);
		p_me->getEnemy()->userInfo->sendMsg(pk2);
	}
}
bool BlessingTree::run()
{
	bool _continue = true;

	matching->EnterCS();

	SendPacket* pk = nullptr;

	deathCount++;
	if (deathCount > db_gimmick[index].data[3] * matching->getMatchingData().frame_rate) {
		type = 0;
		deathCount = 0;
		activity = true;

		int field_idx = matching->getIndexOf(*this);
		pk = new SendPacket(40);
		(*pk) << GAME_PROTOCOL::SERVER_COLLISION_GIMMICK << true << index << field_idx << 2;

		_continue = false;
	}

	matching->LeaveCS();

	if (pk != nullptr) {
		matching->getPlayer1()->userInfo->sendMsg(*pk);
		matching->getPlayer2()->userInfo->sendMsg(*pk);
		delete pk;
	}

	return _continue;
}

void Cat::recv(User& u)// 몽블랑 충돌
{
	matching->EnterCS();

	Player* me = matching->getPlayerOf(&u);
	Player* enemy = me->getEnemy();
	bool hit;

	switch (stat)
	{
	case Cat::STAND:
		hit = matching->Action_reflection_circle(me, position, db_gimmick[index].radius);
		{
			SendPacket sp1(40), sp2(40);
			SendPacket sp3(20), sp4(20);
			SendPacket sp5(40);
			if (hit) {
				totalCount++;
				int myCount;
				if (matching->getPlayer1() == me) {
					p1Count++;
					myCount = p1Count;
				}
				else
					myCount = (totalCount - p1Count);

				sp1 << GAME_PROTOCOL::SERVER_COLLISION_GIMMICK << true << index << matching->getIndexOf(*this) << 0;
				sp1 << myCount << me->crtList[0].direction << me->crtList[0].position;
				sp2 << GAME_PROTOCOL::SERVER_COLLISION_GIMMICK << false << index << matching->getIndexOf(*this) << 0;
				sp2 << myCount << me->crtList[0].direction << me->crtList[0].position;
			}
			// 히트카운트 다 참
			if (totalCount == db_gimmick[index].data[0]) {
				sp3 << GAME_PROTOCOL::SERVER_COLLISION_GIMMICK << true << index << matching->getIndexOf(*this) << 1;
				sp4 << GAME_PROTOCOL::SERVER_COLLISION_GIMMICK << false << index << matching->getIndexOf(*this) << 1;
				wayPointIdx = 0;
				gemCount = 0;
				time = 0;
				direction = (matching->getMatchingData().catWayPoint[0] - position).normalization();
				stat = MOVE;
				home = position;

				sp5 << GAME_PROTOCOL::SERVER_COLLISION_GIMMICK << true << index << matching->getIndexOf(*this) << 2;
				sp5 << direction << position;

				matching->addRuns(this);
			}
			if (hit) {
				me->userInfo->sendMsg(sp1 + sp3 + sp5);
				enemy->userInfo->sendMsg(sp2 + sp4 + sp5);
			}
		}
		break;
	case Cat::MOVE:// 몽블랑이 이동 중 충돌
		break;
	}

	matching->LeaveCS();
}
bool Cat::run()
{
	matching->EnterCS();

	Player* p1 = matching->getPlayer1();
	Player* p2 = matching->getPlayer2();

	position += direction * (db_gimmick[index].data[3] / matching->getMatchingData().frame_rate);

	if (stat == MOVE) {

		float distance = (matching->getMatchingData().catWayPoint[wayPointIdx] - position).size();
		if (distance <= (db_gimmick[index].data[3] / matching->getMatchingData().frame_rate / 2)) {
			wayPointIdx = (++wayPointIdx) % matching->getMatchingData().catWayPoint.size();
			direction = (matching->getMatchingData().catWayPoint[wayPointIdx] - position).normalization();

			SendPacket sp(40);
			sp << GAME_PROTOCOL::SERVER_COLLISION_GIMMICK << true << index << matching->getIndexOf(*this) << 2;
			sp << direction << position;
			p1->userInfo->sendMsg(sp);
			p2->userInfo->sendMsg(sp);
			time = 0;
		}
	}
	else if(stat == COME_BACK){
		float distance = (home - position).size();
		if (distance <= (db_gimmick[index].data[3] / matching->getMatchingData().frame_rate / 2)) {
			position = home;
			totalCount = p1Count = 0;
			stat = STAND;

			SendPacket sp(20);
			sp << GAME_PROTOCOL::SERVER_COLLISION_GIMMICK << true << index << matching->getIndexOf(*this) << 4;
			p1->userInfo->sendMsg(sp);
			p2->userInfo->sendMsg(sp);
			time = 0;
		}
	}

	time++;
	if (time % matching->getMatchingData().frame_rate == 0) {
		SendPacket sp(40);
		sp << GAME_PROTOCOL::SERVER_COLLISION_GIMMICK << true << index << matching->getIndexOf(*this) << 2;
		sp << direction << position;
		p1->userInfo->sendMsg(sp);
		p2->userInfo->sendMsg(sp);
	}

	matching->LeaveCS();
	return (stat == MOVE || stat == COME_BACK);
}
void Cat::recv(User& user, Gem* gem)// 보석 먹음
{
	if (stat != MOVE) return;
	matching->EnterCS();

	if (totalCount / 2.0 < p1Count) {// p1
		if (matching->getPlayer1()->userInfo != &user) {
			matching->LeaveCS();
			return;
		}
	}
	else if (matching->getPlayer2()->userInfo != &user) {// p2
		matching->LeaveCS();
		return;
	}

	bool hit = matching->Action_hitTest_circle(
		position, db_gimmick[index].data[2],
		gem->position, db_gimmick[gem->index].radius);
	if (hit) {// 충돌판정
		Player* me = matching->getPlayerOf(&user);
		Player* enemy = me->getEnemy();

		if (gem->acquire(me)) {// 보석 획득
			gemCount++;

			SendPacket sp(30), sp2(40);
			sp << GAME_PROTOCOL::SERVER_COLLISION_GIMMICK << true << index << matching->getIndexOf(*this) << 3;
			sp << matching->getIndexOf(*gem) << gemCount;

			if (gemCount >= db_gimmick[index].data[1]) {// 원래 자리로 귀환
				stat = COME_BACK;
				direction = (home - position).normalization();

				sp2 << GAME_PROTOCOL::SERVER_COLLISION_GIMMICK << true << index << matching->getIndexOf(*this) << 2;
				sp2 << direction << position;
				time = 0;
			}

			me->sendPlayerDataToMe(PP_SKILLGAUGE | PP_SCORE);
			me->sendPlayerDataToEnemy(PP_SCORE);

			matching->getPlayer1()->userInfo->sendMsg(sp + sp2);
			matching->getPlayer2()->userInfo->sendMsg(sp + sp2);
		}
	}

	matching->LeaveCS();
}