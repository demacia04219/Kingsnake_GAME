#include "MatchingManager.h"
#include "Global.h"
#include "DataBase.h"
#include <math.h>
#include "DebugClientManager.h"
#include "Skill.h"
#include "Item.h"
#include "GameObject.h"
#include <time.h>
#include "Encoding.h"
#include <iostream>

MatchingManager* MatchingManager::Instance = nullptr;

MatchingManager::MatchingManager()
{
}
MatchingManager::~MatchingManager()
{
}

// Singleton Sector
void MatchingManager::CreateInstance()
{
	if (Instance == nullptr)
	{
		Instance = new MatchingManager();
	}
}
MatchingManager* MatchingManager::GetInstance()
{
	return Instance;
}
void MatchingManager::DestroyInstance()
{
	if (Instance != nullptr)
	{
		delete Instance;
	}
}

// GameManager Sector
Matching* MatchingManager::matching(User* user)
{
	DataBaseManager::GetInstance()->EnterCS();
	if (user->getMatching() != nullptr) {
		DataBaseManager::GetInstance()->LeaveCS();
		return user->getMatching();
	} else
		user->setState(USER_STATE::MATCHING);

	int count = DataBaseManager::GetInstance()->getUserCount();
	for (int i = 0; i < count; i++)
	{
		User* user1;
		user1 = DataBaseManager::GetInstance()->getUser(i);
		if (user1 == user)
		{
			continue;
		}
		if (user1->isStateOf(USER_STATE::MATCHING) 
			&& user1->getMatching() == nullptr)
		{
			Matching* matching = new Matching(user1, user, MATCHING_TYPE::BASIC);
			user->setMatching(matching);
			user1->setMatching(matching);

			SendPacket p1(60), p2(60);

			UserData& d = user->getUserData();
			UserData& d2 = user1->getUserData();
			wchar_t *p1_nick, *p2_nick;

			Encoding::AsciiToUnicode(d.nick.data(), p1_nick);
			p1 << SERVER_REQ_MATCHING << true << p1_nick;
			p1 << d.character.leaderCostume << d.character.subCharacterSlot[0] << d.character.subCharacterSlot[1] << d.character.subCharacterSlot[2];
			p1 << d.item.itemSlot[0] << d.item.itemSlot[1] << d.item.itemSlot[2];

			Encoding::AsciiToUnicode(d2.nick.data(), p2_nick);
			p2 << SERVER_REQ_MATCHING << false << p2_nick;
			p2 << d2.character.leaderCostume << d2.character.subCharacterSlot[0] << d2.character.subCharacterSlot[1] << d2.character.subCharacterSlot[2];
			p2 << d2.item.itemSlot[0] << d2.item.itemSlot[1] << d2.item.itemSlot[2];
			

			delete p1_nick;
			delete p2_nick;

			user1->sendMsg(p1); 
			user->sendMsg(p2);

			DataBaseManager::GetInstance()->addMatching(matching);
			DataBaseManager::GetInstance()->LeaveCS();

			matching->StartGame();

			return matching;
		}
	}

	DataBaseManager::GetInstance()->LeaveCS();
	return nullptr;
}
void MatchingManager::recv(User* user)
{
	// 프로토콜 추출											
	GAME_PROTOCOL protocol;
	*user >> protocol;

	if (protocol == GAME_PROTOCOL::CLIENT_CANCEL_MATCHING) {
		recvCancelMatching(user);
		return;
	}
	if (user->getMatching() == nullptr) return;

	// 프로토콜 판별
	switch (protocol)
	{
	case GAME_PROTOCOL::CLIENT_JOYSTICK:
		recvJoystick(user);
		break;
	case GAME_PROTOCOL::CLIENT_COLLISION_WALL:
		recvCollisionWall(user);
		break;
	case GAME_PROTOCOL::CLIENT_COLLISION_OBSTACLE:
		recvCollisionObstacle(user);
		break;
	case GAME_PROTOCOL::CLIENT_COLLISION_GIMMICK:
		recvCollisionGimmick(user);
		break;
	case GAME_PROTOCOL::CLIENT_COLLISION_CAT:
		recvCollisionCat(user);
		break;
	case GAME_PROTOCOL::CLIENT_ATTACK_HIT:
		recvAttackHit(user);
		break;
	case GAME_PROTOCOL::CLIENT_ACTIVE_MOVE:
		recvActiveMove(user);
		break;
	case GAME_PROTOCOL::CLIENT_ACTIVE_ATTACK:
		recvActiveAttack(user);
		break;
	case GAME_PROTOCOL::CLIENT_ACTIVE_GUARD:
		recvActiveGuard(user);
		break;
	case GAME_PROTOCOL::CLIENT_PONG:
		recvPong(user);
		break;
	case GAME_PROTOCOL::CLIENT_TEMPLE_ENTER:
		recvTempleEnter(user);
		break;
	case GAME_PROTOCOL::CLIENT_TEMPLE_CASTING:
		recvTempleCasting(user);
		break;
	case GAME_PROTOCOL::CLIENT_SKILL:
		recvSkill(user);
		break;
	case GAME_PROTOCOL::CLIENT_ITEMSKILL:
		recvItemSkill(user);
		break;
	case GAME_PROTOCOL::CLIENT_GAMEOBJECT:
		recvGameObject(user);
		break;
	case GAME_PROTOCOL::CLIENT_READY:
		recvReady(user);
		break;
	default:
		break;
	}
}

void MatchingManager::disconnect(User* user)
{
	Matching* m = user->getMatching();

	if (DataBaseManager::GetInstance()->removeMatching(m))
	{
		m->EndGame(user);
		delete m;
		printf("Matching was canceled normally\n");
	}
}

void MatchingManager::recvCollisionWall(User* user)
{
	Matching* matching = user->getMatching();
	matching->EnterCS();

	Player* player = matching->getPlayerOf(user);

	Vector2& position = matching->getPlayerOf(user)->crtList[0].position;
	Vector2& direction = matching->getPlayerOf(user)->crtList[0].direction;
	Rect field = matching->getMatchingData().map_size;
	bool hit = false;

	hit = matching->Action_reflection_Wall(player);

	//printf("wall collision%d 충돌여부(%d)\n", (matching->getPlayer1()->userInfo == user ? 1 : 2), hit);

	SendPacket packet(40), packet2(40);

	packet << GAME_PROTOCOL::SERVER_COLLISION_WALL << true;
	packet << matching->getPlayerOf(user)->crtList[0].direction;
	packet << matching->getPlayerOf(user)->crtList[0].position;

	User* enemy = matching->getEnemyOf(user)->userInfo;
	packet2 << GAME_PROTOCOL::SERVER_COLLISION_WALL << false;
	packet2 << matching->getPlayerOf(user)->crtList[0].direction;
	packet2 << matching->getPlayerOf(user)->crtList[0].position;

	matching->LeaveCS();

	user->sendMsg(packet);
	enemy->sendMsg(packet2);


	{
		char msg[100];
		sprintf(msg, "벽 충돌 / 충돌여부(%d)", hit);
		DebugClientManager::GetInstance()->sendEvent(matching, player, DEBUG_PROTOCOL::SERVER_EVENT_COLLISION_WALL, msg);
	}
}
void MatchingManager::recvJoystick(User* user)
{
	Matching* matching = user->getMatching();
	float joystick;

	*user >> joystick;

	matching->EnterCS();

	if (joystick != -999) {
		joystick = ((int)joystick + 180) % 360 - 180;

		// 저주(조이스틱 조작 방향 반대)
		if (matching->getPlayerOf(user)->ability.curse > 0) {
			joystick += 180;
			joystick = ((int)joystick + 180) % 360 - 180;
		}
	}
	
	if (matching->getPlayerOf(user)->canControl() == false) {
		matching->getPlayerOf(user)->joystick = -999;
		matching->LeaveCS();
		return;
	}

	matching->getPlayerOf(user)->joystick = joystick;
	matching->LeaveCS();

	/*if (joystick != -999)
		printf("press joystick%d = %f\n",
		(matching->getPlayer1()->userInfo == user ? 1 : 2), joystick);
	else
		printf("release joystick%d\n",
		(matching->getPlayer1()->userInfo == user ? 1 : 2));*/

	User* enemy = matching->getEnemyOf(user)->userInfo;
	SendPacket packet(20);

	packet << GAME_PROTOCOL::SERVER_JOYSTICK << true << joystick;
	user->sendMsg(packet);

	packet.initBuffer();
	packet << GAME_PROTOCOL::SERVER_JOYSTICK << false << joystick;
	enemy->sendMsg(packet);
}
void MatchingManager::recvAttackHit(User* user)
{
	Matching* matching = user->getMatching();

	int index;
	*user >> index;

	matching->EnterCS();

	Player* _myplayer = matching->getPlayerOf(user);
	Player* _enemy = matching->getEnemyOf(user);

	bool crtFound;
	Character crt = _enemy->getCharacter(index, crtFound);

	Vector2& ref_my_direction = _myplayer->crtList[0].direction;
	Vector2& ref_my_position = _myplayer->crtList[0].position;

	bool isCollide = matching->Action_hitTest_circle(_myplayer, crt.position, matching->getMatchingData().crt_radius)
		&& (matching->getPlayerOf(user)->isStateOf(PLAYER_STATE::DEATH) == false)
		&& (matching->getEnemyOf(user)->isStateOf(PLAYER_STATE::DEATH) == false);
	bool attacked = false;		// 공격
	bool counterattack = false;	// 반격(= 상대방 리더의 공격)
	bool sendEnemyAttack = false;	// 두 리더 충돌 여부


	// 충돌 후 반사각 구하기
	if (isCollide && crtFound)
	{
		if (_myplayer->isStateOf(PLAYER_STATE::GUARD)) {
			if (_enemy->isStateOf(PLAYER_STATE::GUARD)) {
				printf("Attack Hit Error_1\n");
				// 응~ 될 일이 없어~
			}
			else {// me : guard, enemy : not guard
				printf("Attack Hit Error_2\n");
				//matching->Action_reflection_circle(_enemy, _myplayer->crtList[index].position, db_matching.crt_radius);
			}
		}

		else {
			if (_enemy->isStateOf(PLAYER_STATE::GUARD)) {// me : not guard, enemy : guard
				//matching->Action_reflection_circle(_myplayer, _enemy->crtList[index].position, db_matching.crt_radius);
				matching->Action_reflection_circle(_myplayer, crt.position, matching->getMatchingData().crt_radius);
			}
			else {// me : not guard, enemy : not guard
				if (crt.index == 0)
					matching->Action_reflection_player(_myplayer, _enemy);
				else
					//matching->Action_reflection_circle(_myplayer, _enemy->crtList[index].position, db_matching.crt_radius);
					matching->Action_reflection_circle(_myplayer, crt.position, matching->getMatchingData().crt_radius);
			}
		}

		attacked = _myplayer->attack(matching->getEnemyOf(user), _enemy->getCharacterIndex(index));
		if (attacked) {
			_myplayer->ability.addSkillGauge(_myplayer->ability.getAttackRecovery());
			_myplayer->sendPlayerDataToMe(PP_SKILLGAUGE);
		}

		if (crt.index == 0) {
			counterattack = _enemy->attack(matching->getPlayerOf(user), 0);
			sendEnemyAttack = true;

			if (counterattack) {
				_enemy->ability.addSkillGauge(_enemy->ability.getAttackRecovery());
				_enemy->sendPlayerDataToMe(PP_SKILLGAUGE);
			}
		}
	}

	// 기본공격 패킷 보내기
	//printf("player%d collided with enemy[idx:%d, hp:%f]. 충돌여부(%d), 공격여부(%d), 반격여부(%d)\n", matching->getPlayer1()->userInfo == user ? 1 : 2, index, matching->getEnemyOf(user)->crtList[index].hp, isCollide, attacked, counterattack);
	SendPacket pk(30), pk2(30), pk3(30), pk4(30);
	Player* enemy_player = matching->getEnemyOf(user);
	User* enemy = enemy_player->userInfo;
	crt = _enemy->getCharacter(index, crtFound);

	pk << GAME_PROTOCOL::SERVER_ATTACK_HIT << false;
	pk << enemy_player->crtList[0].direction << enemy_player->crtList[0].position;
	pk << index << crt.hp;

	pk2 << GAME_PROTOCOL::SERVER_ATTACK_HIT << true;
	pk2 << enemy_player->crtList[0].direction << enemy_player->crtList[0].position;
	pk2 << index << crt.hp;

	pk3 << GAME_PROTOCOL::SERVER_ATTACK_HIT << true;
	pk3 << ref_my_direction << ref_my_position << 0 << ((Character)_myplayer->getCharacter(0, crtFound)).hp;

	pk4 << GAME_PROTOCOL::SERVER_ATTACK_HIT << false;
	pk4 << ref_my_direction << ref_my_position << 0 << ((Character)_myplayer->getCharacter(0, crtFound)).hp;
	
	matching->LeaveCS();

	user->sendMsg(pk + pk3);
	enemy->sendMsg(pk2 + pk4);

	// 디버깅 클라
	DebugClientManager::GetInstance()->sendChangeTemple(matching);
	if (sendEnemyAttack) {// 상대의 반격 이벤트
		DebugClientManager::GetInstance()->sendHitAttack(matching, true, counterattack, _myplayer);
	}
	// 나의 공격 이벤트
	DebugClientManager::GetInstance()->sendHitAttack(matching, isCollide, attacked, _enemy);
}
void MatchingManager::recvCollisionObstacle(User* user)
{
	Matching* matching = user->getMatching();

	// 클라로부터 인덱스 받음
	int index;
	*user >> index;

	matching->EnterCS();

	if (index >= matching->getMatchingData().obstacle.size()) {
		matching->LeaveCS();
		return;
	}

	Vector2& ref_my_direction = matching->getPlayerOf(user)->crtList[0].direction;
	Vector2& ref_my_position = matching->getPlayerOf(user)->crtList[0].position;

	Vector2 obstacle_position = matching->getMatchingData().obstacle[index]->position;

	// 충돌 판정 및 반사
	bool isCollide = matching->Action_reflection_circle(matching->getPlayerOf(user), obstacle_position, matching->getMatchingData().obstacle_radius);

	SendPacket pk1(25), pk2(25);
	User* enemy = matching->getEnemyOf(user)->userInfo;

	pk1 << GAME_PROTOCOL::SERVER_COLLISION_OBSTACLE << true;
	pk1 << ref_my_direction;
	pk1 << ref_my_position;
	pk2 << GAME_PROTOCOL::SERVER_COLLISION_OBSTACLE << false;
	pk2 << ref_my_direction;
	pk2 << ref_my_position;

	matching->LeaveCS();

	user->sendMsg(pk1);
	enemy->sendMsg(pk2);

	{
		char msg[100];
		sprintf(msg, "장애물 충돌 / 충돌여부(%d)", isCollide);
		DebugClientManager::GetInstance()->sendEvent(matching, matching->getPlayerOf(user), DEBUG_PROTOCOL::SERVER_EVENT_COLLISION_OBSTACLE, msg);
	}
	//printf("obstacle collision%d 충돌여부(%d) index(%d)\n", (matching->getPlayer1()->userInfo == user ? 1 : 2), isCollide, index);
}
void MatchingManager::recvCollisionCat(User* user)
{
	Matching* matching = user->getMatching();
	int gemIdx;

	*user >> gemIdx;

	matching->EnterCS();

	if (matching->getGimmick(gemIdx).index != MAP_OBJECT_TYPE::GIMMICK_GEM) {
		printf("Collision Cat Error : Index(%d) is not Gem Index\n", gemIdx);// 인덱스 범위 초과
		matching->LeaveCS();
		return;
	}

	Gem* gem = (Gem*)(&matching->getGimmick(gemIdx));
	matching->LeaveCS();

	matching->getCat().recv(*user, gem);
}
void MatchingManager::recvCollisionGimmick(User* user)
{
	Matching* matching = user->getMatching();

	int index;
	*user >> index;

	matching->EnterCS();

	if (matching->getGimmick(index).index == MAP_OBJECT_TYPE::IDLE) {
		printf("Collision Gimmick Error : Exceeded field object index\n");// 인덱스 범위 초과
		matching->LeaveCS();
		return;
	}
	
	Gimmick& g = matching->getGimmick(index);
	matching->LeaveCS();

	g.recv(*user);
}
void MatchingManager::recvPong(User* user)
{
	user->setPong();
	Player *p = user->getMatching()->getPlayerOf(user);
	DebugClientManager::GetInstance()->sendPing(p->matching, p);
}

void MatchingManager::recvActiveMove(User* user)
{
	Matching* matching = user->getMatching();
	matching->EnterCS();

	Player* me = matching->getPlayerOf(user);

	if (me->isStateOf(PLAYER_STATE::DEATH) == false &&
		me->canControl())
	{
		me->ability.speedPer = 1;
		me->setState(PLAYER_STATE::MOVE);
		//printf("player%d change battle mode = move\n", matching->getPlayer1()->userInfo == user ? 1 : 2);

		SendPacket pk1(10), pk2(10);
		User* enemy = matching->getEnemyOf(user)->userInfo;

		pk1 << GAME_PROTOCOL::SERVER_ACTIVE_MOVE << true << me->ability.getSpeed();
		pk2 << GAME_PROTOCOL::SERVER_ACTIVE_MOVE << false << me->ability.getSpeed();

		matching->LeaveCS();

		user->sendMsg(pk1);
		enemy->sendMsg(pk2);
		me->sendPlayerDataToMe(PP_STAMINA);

	}
	else
		matching->LeaveCS();
}
void MatchingManager::recvActiveAttack(User* user)
{
	Matching* matching = user->getMatching();
	matching->EnterCS();
	Player& me = *(matching->getPlayerOf(user));

	if (matching->getPlayerOf(user)->isStateOf(PLAYER_STATE::DEATH) == false &&
		matching->getPlayerOf(user)->canControl() && me.ability.getStamina() > 10)
	{
		matching->getPlayerOf(user)->ability.speedPer = 2;
		matching->getPlayerOf(user)->setState(PLAYER_STATE::ATTACK);
		//printf("player%d change battle mode = attack\n", matching->getPlayer1()->userInfo == user ? 1 : 2);

		SendPacket pk1(10), pk2(10);
		User* enemy = matching->getEnemyOf(user)->userInfo;
		pk1 << GAME_PROTOCOL::SERVER_ACTIVE_ATTACK << true << matching->getPlayerOf(user)->ability.getSpeed();
		pk2 << GAME_PROTOCOL::SERVER_ACTIVE_ATTACK << false << matching->getPlayerOf(user)->ability.getSpeed();

		matching->LeaveCS();

		user->sendMsg(pk1);
		enemy->sendMsg(pk2);
		me.sendPlayerDataToMe(PP_STAMINA);
	}
	else
		matching->LeaveCS();
}
void MatchingManager::recvActiveGuard(User* user)
{
	Matching* matching = user->getMatching();
	matching->EnterCS();
	Player& me = *(matching->getPlayerOf(user));

	if (matching->getPlayerOf(user)->isStateOf(PLAYER_STATE::DEATH) == false &&
		matching->getPlayerOf(user)->canControl(ALLOW_CASTING) && me.ability.getStamina() > 10)
	{
		matching->getPlayerOf(user)->ability.speedPer = 0;
		matching->getPlayerOf(user)->setState(PLAYER_STATE::GUARD);
		//printf("player%d change battle mode = guard\n", matching->getPlayer1()->userInfo == user ? 1 : 2);

		SendPacket pk1(10), pk2(10);
		User* enemy = matching->getEnemyOf(user)->userInfo;
		Temple& temple = matching->getTemple();


		pk1 << GAME_PROTOCOL::SERVER_ACTIVE_GUARD << true << matching->getPlayerOf(user)->ability.getSpeed();
		pk2 << GAME_PROTOCOL::SERVER_ACTIVE_GUARD << false << matching->getPlayerOf(user)->ability.getSpeed();
		

		// 캐스팅 중 가드로 전환 시 캐스팅 해제.
		SendPacket pk3(30), pk4(30), pk5(10), pk6(10);
		bool cancelCasting = false;
		bool addSend = false;
		if (temple.casting) {
			Player& p = *(matching->getPlayerOf(user));
			Player& e = *(matching->getEnemyOf(user));
			if (temple.firstPlayer == &p) {
				cancelCasting = true;
				temple.tryToCasting(p, e, false, false);
				p.ability.speedPer = 0;
				p.setState(PLAYER_STATE::GUARD, false);

				pk3 << GAME_PROTOCOL::SERVER_TEMPLE_CASTING << true << false;
				pk3 << p.ability.getSpeed() << temple.gauge << true;

				pk4 << GAME_PROTOCOL::SERVER_TEMPLE_CASTING << false << false;
				pk4 << p.ability.getSpeed() << temple.gauge << true;

				if (temple.allIntoTemple) {
					addSend = true;
					pk5 << GAME_PROTOCOL::SERVER_TEMPLE_ENTER << false << temple.gauge;
					pk6 << GAME_PROTOCOL::SERVER_TEMPLE_ENTER << false << temple.gauge;
				}
			}
		}

		matching->LeaveCS();

		user->sendMsg(pk1);
		enemy->sendMsg(pk2);
		me.sendPlayerDataToMe(PP_STAMINA);

		char msg[200];
		
		if (cancelCasting) {
			sprintf(msg, "템플 캐스팅 종료(가드)");
			user->sendMsg(pk3);
			enemy->sendMsg(pk4);
			if (addSend) {
				sprintf(msg, "템플 캐스팅 종료(가드), 동시 Enter(버튼 생성X)");
				user->sendMsg(pk5);
				enemy->sendMsg(pk6);
			}
			DebugClientManager::GetInstance()->sendEvent(matching, matching->getPlayerOf(user), DEBUG_PROTOCOL::SERVER_EVENT_TEMPLE, msg);
		}

	}
	else
		matching->LeaveCS();
}
void MatchingManager::recvTempleEnter(User* user)
{
	Matching* matching = user->getMatching();

	bool enter;
	char msg[100];
	*user >> enter;

	matching->EnterCS();

	Player& player = *(matching->getPlayerOf(user));
	Temple& temple = matching->getTemple();
	SendPacket packet(10), pk2(10);
	bool enemySend = false;

	if (enter) {
		bool can = temple.enter(player, true);
		packet << GAME_PROTOCOL::SERVER_TEMPLE_ENTER << can << temple.gauge;
		sprintf(msg, "템플 Enter(%d)", enter);
	}
	else {
		bool canCancel = temple.enter(player, false);
		packet << GAME_PROTOCOL::SERVER_TEMPLE_ENTER << (!canCancel) << temple.gauge;
		sprintf(msg, "템플 Leave(%d)", canCancel);

		if (canCancel && temple.cooltime == 0) {
			if(matching->Action_hitTest_circle(matching->getEnemyOf(user),temple.position, db_gimmick[temple.index].radius)){
				enemySend = true;
				pk2 << GAME_PROTOCOL::SERVER_TEMPLE_ENTER << true << temple.gauge;
			}
		}
		if (!enemySend) {
			enemySend = true;
			pk2 << GAME_PROTOCOL::SERVER_TEMPLE_ENTER << temple.isCanCasting(player.getEnemy()) << temple.gauge;
		}
	}

	matching->LeaveCS();

	user->sendMsg(packet);
	if (enemySend)
		matching->getEnemyOf(user)->userInfo->sendMsg(pk2);

	DebugClientManager::GetInstance()->sendChangeTemple(matching);
	DebugClientManager::GetInstance()->sendEvent(matching, &player, DEBUG_PROTOCOL::SERVER_EVENT_TEMPLE, msg);
}
void MatchingManager::recvTempleCasting(User* user)
{
	Matching* matching = user->getMatching();

	bool start;
	char msg[200];
	*user >> start;

	matching->EnterCS();
	Player& player = *(matching->getPlayerOf(user));
	Temple& temple = matching->getTemple();
	User* enemy = matching->getEnemyOf(user)->userInfo;

	SendPacket pk1(30), pk2(30), pk3(10), pk4(10);
	bool addSend = false;

	if (start && player.canControl() && player.ability.getStamina() > 10)
	{
		bool canStart = temple.tryToCasting(player, *matching->getEnemyOf(user), true);
		sprintf(msg, "템플 캐스팅 시작(%d)", canStart);

		pk1 << GAME_PROTOCOL::SERVER_TEMPLE_CASTING << true << canStart;
		pk1 << player.ability.getSpeed() << temple.gauge << false;

		pk2 << GAME_PROTOCOL::SERVER_TEMPLE_CASTING << false << canStart;
		pk2 << player.ability.getSpeed() << temple.gauge << false;
		
		player.sendPlayerDataToMe(PP_STAMINA);
	}
	else {
		temple.tryToCasting(player, *matching->getEnemyOf(user), false);

		pk1 << GAME_PROTOCOL::SERVER_TEMPLE_CASTING << true << false;
		pk1 << player.ability.getSpeed() << temple.gauge << false;

		pk2 << GAME_PROTOCOL::SERVER_TEMPLE_CASTING << false << false;
		pk2 << player.ability.getSpeed() << temple.gauge << false;

		if(temple.allIntoTemple)
			sprintf(msg, "템플 캐스팅 종료(1), 동시 Enter(버튼 생성X)");
		else
			sprintf(msg, "템플 캐스팅 종료(1)");

		if (temple.allIntoTemple) {
			addSend = true;
			pk3 << GAME_PROTOCOL::SERVER_TEMPLE_ENTER << false << temple.gauge;
			pk4 << GAME_PROTOCOL::SERVER_TEMPLE_ENTER << false << temple.gauge;
		}
		player.sendPlayerDataToMe(PP_STAMINA);
	}

	matching->LeaveCS();

	user->sendMsg(pk1);
	enemy->sendMsg(pk2);
	if (addSend) {
		user->sendMsg(pk3);
		enemy->sendMsg(pk4);
	}

	DebugClientManager::GetInstance()->sendChangeTemple(matching);
	DebugClientManager::GetInstance()->sendEvent(matching, &player, DEBUG_PROTOCOL::SERVER_EVENT_TEMPLE, msg);
}


void MatchingManager::recvSkill(User* user)
{
	Matching* matching = user->getMatching();

	int slot;
	*user >> slot;
	if (slot < 0 || slot > 2)	return;

	matching->EnterCS();
	bool found;
	if (matching->getPlayerOf(user)->getCharacter(slot + 1, found).hp == 0) {
		matching->LeaveCS();
		return;
	}

	Skill* skill = matching->getPlayerOf(user)->skills[slot];

	matching->LeaveCS();
	
	if(skill != nullptr)
		skill->recv();
}
void MatchingManager::recvItemSkill(User* user)
{
	Matching* matching = user->getMatching();

	int slot;
	*user >> slot;
	if (slot < 0 || slot > 2)	return;

	matching->EnterCS();

	Item* item = matching->getPlayerOf(user)->items[slot];

	matching->LeaveCS();

	if(item != nullptr)
		item->recv();
}
void MatchingManager::recvGameObject(User* user)
{
	Matching* matching = user->getMatching();

	int serial;		// 오브젝트 고유 번호
	*user >> serial;

	matching->EnterCS();

	GameObject* obj = matching->getGameObject(serial);

	matching->LeaveCS();
	
	if (obj != nullptr) {
		obj->recv();
	}
	else {
		printf("error recvGameObject() : serial number is invalid\n");
	}
}
void MatchingManager::recvReady(User* user)
{
	Matching* matching = user->getMatching();

	matching->EnterCS();
	matching->setReady(user);
	matching->LeaveCS();
}
void MatchingManager::recvCancelMatching(User* user)
{
	DataBaseManager::GetInstance()->EnterCS();

	if (user->isStateOf(USER_STATE::MATCHING) && user->getMatching() == nullptr)
	{
		user->setState(USER_STATE::LOBBY);
	}

	DataBaseManager::GetInstance()->LeaveCS();
}