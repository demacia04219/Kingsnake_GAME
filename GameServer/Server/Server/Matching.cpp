#include "Matching.h"
#include <math.h>
#include "Global.h"
#include <time.h>
#include "DebugClientManager.h"
#include "DataBase.h"
#include "DynamicGameData.h"
#include <functional>
#include "Skill.h"
#include "Item.h"
#include "GameObject.h"
#include "DBServerManager.h"

enum PLAYERINDEX
{
	P1 = 0,
	P2
};
enum CHARACTERINDEX
{
	LEADER = 0,
	SUB1, SUB2, SUB3
};

Matching::Matching(User* user1, User* user2, MATCHING_TYPE type)
{
	static int _serial = 0;
	serial = ++_serial;
	InitializeCriticalSection(&cs);
	count = 0;
	this->type = type;
	matchingData = db_matching[type];
	canDelete = CreateEvent(0, true, false, 0);
	progress = PG_PREPARE;

	p[P1] = new Player(user1, this);
	p[P2] = new Player(user2, this);
	protectDelete[P1] = CreateEvent(0, true, true, 0);
	protectDelete[P2] = CreateEvent(0, true, true, 0);
	//p[P1]->init();
	//p[P2]->init();
	
	int gimmickCount = matchingData->gimmick.size();
	for (int i = 0; i < gimmickCount; i++) {
		//gimmicks.push_back(*db_matching.gimmick[i]);
		Gimmick* g = Gimmick::createGimmick(matchingData->gimmick[i]->index, matchingData->gimmick[i]);
		g->matching = this;
		gimmicks.push_back(g);

		if (g->index == MAP_OBJECT_TYPE::GIMMICK_TEMPLE) {
			temple = dynamic_cast<Temple*>(g);
			if (temple == nullptr) {
				throw "???";
			}
			runGimmicks.push_back(temple);
		}
		if (g->index == MAP_OBJECT_TYPE::GIMMICK_CAT) {
			cat = dynamic_cast<Cat*>(g);
			if (cat == nullptr)
				throw "???";
		}
	}

	DataBaseManager::GetInstance()->EnterCS();
	Gem::startFrame[serial] = 0;
	DataBaseManager::GetInstance()->LeaveCS();

	int n = Gem::startFrame[serial];

	DebugClientManager::GetInstance()->sendCreateMatching(this);
}
Matching::~Matching()
{
	DebugClientManager::GetInstance()->sendDestroyMatching(this);
	DeleteCriticalSection(&cs);

	DataBaseManager::GetInstance()->EnterCS();
	Gem::startFrame.erase(serial);
	DataBaseManager::GetInstance()->LeaveCS();

	CloseHandle(canDelete);
	CloseHandle(protectDelete[0]);
	CloseHandle(protectDelete[1]);

	delete p[0];
	delete p[1];

	int count = gimmicks.size();
	for (int i = 0; i < count; i++)
		delete gimmicks[i];

	for (auto obj : gameObjects)
		delete obj.second;
}

// 게임 시작 함수
void Matching::StartGame()
{
	printf("Matching start\n");

	EnterCS();
	if (DataBaseManager::GetInstance()->contain(this) == false) {
		LeaveCS();
		SetEvent(canDelete);
		return;
	}

	//디버그용
	if(DEBUG_MATCHING)
	{
		SendPacket sp;
		sp << SERVER_GAMESTART;
		p[P1]->userInfo->sendMsg(sp);
		p[P2]->userInfo->sendMsg(sp);
	}

	
	tickTime = 1000.0 / matchingData->frame_rate;
	tickTime += (serial % 8) / 10000.0;// 타이머의 쓰레드를 멀티쓰레드화 하기 위해 약간의 오차를 만듦
	TimerManager::GetInstance()->addTimer(this, tickTime);

	LeaveCS();
	SetEvent(canDelete);
}
void Matching::EndGame(User* disconnectUser)
{
	WaitForSingleObject(canDelete, INFINITE);
	TimerManager::GetInstance()->removeTimer(this, tickTime);

	DataBaseManager* db = DataBaseManager::GetInstance();

	if (db->contain(p[P1]->userInfo)) {
		WaitProtectDelete();// 다른 쓰레드의 recv함수에서 반환된 matching에 접근하는 것을 막기 위해
		p[P1]->userInfo->setState(USER_STATE::LOBBY);
	}
	if (db->contain(p[P2]->userInfo)) {
		WaitProtectDelete();// 다른 쓰레드의 recv함수에서 반환된 matching에 접근하는 것을 막기 위해
		p[P2]->userInfo->setState(USER_STATE::LOBBY);
	}

	db->EnterCS();
	int p1_extraExp,  p2_extraExp;
	int p1_extraGold, p2_extraGold;
	int p1_extraScore,p2_extraScore;
	int p1_result, p2_result;// 0 승, 1 패, 2 무승부
	UserBasicData& d1 = p[P1]->userInfo->getUserData().basic;
	UserBasicData& d2 = p[P2]->userInfo->getUserData().basic;

	SendPacket sp_db(50);// DB에게 보내는 패킷
	sp_db << DB_PROTOCOL::GAMESERVER_MATCHEND << p[P1]->userInfo->getUserData().id << p[P2]->userInfo->getUserData().id;

	// p1 승
	if (p[P1]->score > p[P2]->score || p[P2]->userInfo == disconnectUser) {
		d1.victoryCount++;
		d1.victoryStreak++;
		d1.loseStreak = 0;
		d2.loseStreak++;
		d2.victoryStreak = 0;

		p1_result = 0; p2_result = 1;
		p1_extraExp = 50; p2_extraExp = 10;
		p1_extraGold = 100; p2_extraGold = 30;
		p1_extraScore = d1.victoryStreak;
		p2_extraScore = (d2.loseStreak <= d2.score? d2.loseStreak : d2.score);
		sp_db << 0;
	}
	// p2 승
	else if (p[P1]->score < p[P2]->score || p[P1]->userInfo == disconnectUser) {
		d1.loseStreak++;
		d1.victoryStreak = 0;
		d2.victoryCount++;
		d2.victoryStreak++;
		d2.loseStreak = 0;

		p1_result = 1; p2_result = 0;
		p1_extraExp = 10; p2_extraExp = 50;
		p1_extraGold = 30; p2_extraGold = 100;
		p2_extraScore = d2.victoryStreak;
		p1_extraScore = (d1.loseStreak <= d1.score ? d1.loseStreak : d1.score);
		sp_db << 1;
	}
	else {// 무승부
		d1.victoryStreak = d1.loseStreak = 0;
		d2.victoryStreak = d2.loseStreak = 0;

		p1_result = p2_result = 2;
		p1_extraExp = 30; p2_extraExp = 30;
		p1_extraGold = 50; p2_extraGold = 50;
		p1_extraScore = 1;
		p2_extraScore = 1;
		sp_db << 3;
	}

	d1.matchingCount++;
	d2.matchingCount++;
	d1.exp += p1_extraExp;
	d2.exp += p2_extraExp;
	d1.gold += p1_extraGold;
	d2.gold += p2_extraGold;
	d1.score += p1_extraScore;
	d2.score += p2_extraScore;
	if (d1.exp >= d1.lv * 100) {
		d1.exp -= d1.lv * 100;
		d1.lv++;
	}
	if (d2.exp >= d2.lv * 100) {
		d2.exp -= d2.lv * 100;
		d2.lv++;
	}

	SendPacket sp1(40), sp2(40);
	sp1 << SERVER_GAMEFINISH << p1_result << d1.lv << p1_extraExp << d1.exp << p1_extraGold << d1.gold << p1_extraScore << d1.score;
	sp2 << SERVER_GAMEFINISH << p2_result << d2.lv << p2_extraExp << d2.exp << p2_extraGold << d2.gold << p2_extraScore << d2.score;
	p[P1]->userInfo->sendMsg(sp1);
	p[P2]->userInfo->sendMsg(sp2);

	DBServerManager::GetInstance()->sendSetBasicInfo(*(p[P1]->userInfo));
	DBServerManager::GetInstance()->sendSetMatchInfo(*(p[P1]->userInfo));
	DBServerManager::GetInstance()->sendSetBasicInfo(*(p[P2]->userInfo));
	DBServerManager::GetInstance()->sendSetMatchInfo(*(p[P2]->userInfo));
	DBServerManager::GetInstance()->getSocket().sendMsg(sp_db);

	db->LeaveCS();

	p[P1]->userInfo = nullptr;
	p[P2]->userInfo = nullptr;
}
Player* Matching::getPlayer1()
{
	return p[P1];
}
Player* Matching::getPlayer2()
{
	return p[P2];
}
Player* Matching::getPlayerOf(User* user)
{
	if (p[P1]->userInfo == user)
		return p[P1];
	else
		return p[P2];
}
Temple& Matching::getTemple()
{
	return *temple;
}
Cat& Matching::getCat()
{
	return *cat;
}
Player* Matching::getPlayer(int index)
{
	if (index < 0 || index > 1)
		return nullptr;
	else
		return p[index];
}
Player* Matching::getEnemyOf(User* user)
{
	if (p[P1]->userInfo == user)
		return p[P2];
	else
		return p[P1];
}
int Matching::getSerial()
{
	return serial;
}
unsigned int Matching::getCurrentFrame()
{
	if (progress == PG_GAME)
		return count;
	else
		return 0;
}
int Matching::getGimmicksCount()
{
	return gimmicks.size();
}
MatchingData& Matching::getMatchingData()
{
	return *matchingData;
}
Gimmick& Matching::getGimmick(int index)
{
	if (index >= gimmicks.size()) {
		Gimmick g;
		g.index = MAP_OBJECT_TYPE::IDLE;
		return g;
	}
	return *(gimmicks[index]);
}
int Matching::getIndexOf(Gimmick& g)
{
	EnterCS();
	int count = gimmicks.size();
	for (int i = 0; i < count; i++) {
		if (gimmicks[i] == &g) {
			LeaveCS();
			return i;
		}
	}
	LeaveCS();
	return -1;
}
void Matching::addRuns(Gimmick* g)
{
	EnterCS();
	int count = runGimmicks.size();
	for (int i = 0; i < count; i++) {
		if (runGimmicks[i] == g) {
			LeaveCS();
			return;
		}
	}
	runGimmicks.push_back(g);
	LeaveCS();
}
void Matching::addRuns(GameObject* g)
{
	EnterCS();
	if (gameObjects[g->getSerial()] != nullptr) {// 키값 찾았을 때
		LeaveCS();
		return;
	}
	gameObjects[g->getSerial()] = g;
	LeaveCS();
}
GameObject* Matching::getGameObject(int serial)
{
	EnterCS();
	GameObject* result = gameObjects[serial];
	LeaveCS();
	return result;
}
void Matching::EnterCS()
{
	EnterCriticalSection(&cs);
}
void Matching::LeaveCS()
{
	LeaveCriticalSection(&cs);
}

/* Refactoring part */
void Matching::processFrame()
{
	getPlayer1()->run();
	getPlayer2()->run();

	/* gimmick run per frame */
	int runCount = runGimmicks.size();
	for (int i = 0; i < runCount; i++) {
		if (runGimmicks[i]->run() == false) {
			runGimmicks.erase(runGimmicks.begin() + i);
			i--;
			runCount--;
		}
	}
	/* player run per frame*/
	for (int i = 0; i < 3; i++) {
		if (p[0]->skills[i] != nullptr && p[0]->skills[i]->isActived())
			p[0]->skills[i]->process();
		if (p[0]->items[i] != nullptr && p[0]->items[i]->isActived())
			p[0]->items[i]->process();
		if (p[1]->skills[i] != nullptr && p[1]->skills[i]->isActived())
			p[1]->skills[i]->process();
		if (p[1]->items[i] != nullptr && p[1]->items[i]->isActived())
			p[1]->items[i]->process();
	}

	// GameObject
	for (auto obj = gameObjects.begin(); obj != gameObjects.end();)
	{
		if (obj->second->isActivited())
			obj->second->process();
		else {
			auto tmp = obj++;
			delete tmp->second;
			gameObjects.erase(tmp->first);
			continue;
		}
		obj++;
	}
}
void Matching::lerp()
{
	Player* playerList[2] = { getPlayer1(), getPlayer2() };
	User* user;
	Player* player;

	for (int j = 0; j < 2; j++) {
		SendPacket packet(50), packet2(50);

		user = playerList[j]->userInfo;
		player = getPlayerOf(user);
		if (player->isStateOf(PLAYER_STATE::DEATH) == false) {
			packet << GAME_PROTOCOL::SERVER_POSITION_PLAYER;
			packet << player->crtCount;
			packet << player->crtList[0].direction;
			for (int i = 0; i < player->crtCount; i++) {
				packet << player->crtList[i].position;
			}
		}

		player = getEnemyOf(user);
		if (player->isStateOf(PLAYER_STATE::DEATH) == false) {
			packet2 << GAME_PROTOCOL::SERVER_POSITION_ENEMY;
			packet2 << player->crtCount;
			packet2 << player->crtList[0].direction;
			for (int i = 0; i < player->crtCount; i++) {
				packet2 << player->crtList[i].position;
			}
		}
		user->sendMsg(packet + packet2);
	}
}

bool Matching::isLerpAvailable()
{
	return count % (int)(matchingData->frame_rate) == 0;
}

void Matching::process()
{
	count++;
	if (gameFinish()) return;
	if (prepare()) return;

	processFrame();
	if (isLerpAvailable()) {
		lerp();
	}

	DebugClientManager::GetInstance()->sendMatchingFrame(this);
}

bool Matching::Action_reflection_Wall(FieldObject* obj)
{
	Rect field = matchingData->map_size;
	Vector2& position = obj->position;
	Vector2& direction = obj->direction;
	float radius = obj->getRadius();

	bool hit = false;
	// 벽과 충돌 판정
	if (position.x - radius < field.leftBottom.x) {
		hit = true;
		position.x = field.leftBottom.x + radius;
		direction.x *= -1;
	}
	if (position.x + radius > field.rightTop.x) {
		hit = true;
		position.x = field.rightTop.x - radius;
		direction.x *= -1;
	}
	if (position.y - radius < field.leftBottom.y) {
		hit = true;
		position.y = field.leftBottom.y + radius;
		direction.y *= -1;
	}
	if (position.y + radius > field.rightTop.y) {
		hit = true;
		position.y = field.rightTop.y - radius;
		direction.y *= -1;
	}
	return hit;
}
bool Matching::Action_reflection_Wall(Player* player)
{
	Rect field = matchingData->map_size;
	Vector2& position = player->crtList[0].position;
	Vector2& direction = player->crtList[0].direction;

	bool hit = false;
	// 벽과 충돌 판정
	if (position.x - matchingData->crt_radius < field.leftBottom.x) {
		hit = true;
		position.x = field.leftBottom.x + matchingData->crt_radius;
		direction.x *= -1;
	}
	if (position.x + matchingData->crt_radius > field.rightTop.x) {
		hit = true;
		position.x = field.rightTop.x - matchingData->crt_radius;
		direction.x *= -1;
	}
	if (position.y - matchingData->crt_radius < field.leftBottom.y) {
		hit = true;
		position.y = field.leftBottom.y + matchingData->crt_radius;
		direction.y *= -1;
	}
	if (position.y + matchingData->crt_radius > field.rightTop.y) {
		hit = true;
		position.y = field.rightTop.y - matchingData->crt_radius;
		direction.y *= -1;
	}
	return hit;
}
bool Matching::Action_hitTest_wall(Player* _player)
{
	Rect field = matchingData->map_size;
	Vector2 position = _player->crtList[0].position;
	if (position.x - matchingData->crt_radius < field.leftBottom.x) {
		return true;
	}
	if (position.x + matchingData->crt_radius > field.rightTop.x) {
		return true;
	}
	if (position.y - matchingData->crt_radius < field.leftBottom.y) {
		return true;
	}
	if (position.y + matchingData->crt_radius > field.rightTop.y) {
		return true;
	}
	return false;
}
bool Matching::Action_hitTest_box(Vector2& position, float radius, Rect box)
{
	Vector2 center = (box.leftBottom + box.rightTop) * 0.5;
	Vector2 scale = box.rightTop - center;
	Vector2 nearPoint = position;

	if (position.x < center.x - scale.x) {
		nearPoint.x = center.x - scale.x;
	}
	else if (position.x > center.x + scale.x) {
		nearPoint.x = center.x + scale.x;
	}
	if (position.y < center.y - scale.y) {
		nearPoint.y = center.y - scale.y;
	}
	else if (position.y > center.y + scale.y) {
		nearPoint.y = center.y + scale.y;
	}
	bool hit = (nearPoint - position).size() <= radius;
	return hit;
}
bool Matching::Action_reflection_box(Player* player, float radius, Rect box)
{
	Vector2& position = player->crtList[0].position;
	Vector2& speed = player->crtList[0].direction;

	Vector2 center = (box.leftBottom + box.rightTop) * 0.5;
	Vector2 scale = box.rightTop - center;
	Vector2 nearPoint = position;

	if (position.x < center.x - scale.x) {
		nearPoint.x = center.x - scale.x;
	}
	else if (position.x > center.x + scale.x) {
		nearPoint.x = center.x + scale.x;
	}
	if (position.y < center.y - scale.y) {
		nearPoint.y = center.y - scale.y;
	}
	else if (position.y > center.y + scale.y) {
		nearPoint.y = center.y + scale.y;
	}
	bool hit = (nearPoint - position).size() <= radius;
	if (hit) {
		Vector2 normal;

		if (position == nearPoint) {
			if (position.x > center.x)
				position.x = center.x + scale.x;
			else
				position.x = center.x - scale.x;

			if (position.y > center.y)
				position.y = center.y + scale.y;
			else
				position.y = center.y - scale.y;
		}

		normal = position - nearPoint;
		normal = normal.normalization();
		position = nearPoint + normal * radius;

		speed += speed.projection(normal * -1) * -2;
	}
	return hit;
}
bool Matching::Action_hitTest_circle(Vector2 A_position, float A_radius, Vector2 B_position, float B_radius)
{
	float dx, dy, R, D;
	dx = B_position.x - A_position.x;
	dy = B_position.y - A_position.y;
	R = A_radius + B_radius;
	D = dx * dx + dy * dy;
	return (D < R * R);
}
bool Matching::Action_hitTest_circle(Player* player, Vector2 B_position, float B_radius)
{
	Vector2& A_position = player->crtList[0].position;
	float A_radius = matchingData->crt_radius;

	float dx, dy, R, D;
	dx = B_position.x - A_position.x;
	dy = B_position.y - A_position.y;
	R = A_radius + B_radius;
	D = dx * dx + dy * dy;
	return (D < R * R);
}
bool Matching::Action_reflection_player(Player* player, Player* player2)
{
	Vector2& A_position = player->crtList[0].position;
	Vector2& A_speed = player->crtList[0].direction;
	Vector2& B_position = player2->crtList[0].position;
	Vector2& B_speed = player2->crtList[0].direction;
	float A_radius = matchingData->crt_radius;
	float B_radius = matchingData->crt_radius;

	float dx, dy, R, D, cos, sin, Q, cy1, cy2, rx1, rx2, cx1, cx2;
	dx = B_position.x - A_position.x;
	dy = B_position.y - A_position.y;
	R = A_radius + B_radius;
	D = dx * dx + dy * dy;
	if (D < R * R) {
		D = sqrt(D);
		cos = dx / D;
		sin = dy / D;
		Q = R - D;
		A_position.x = A_position.x - Q * cos / 2;
		A_position.y = A_position.y - Q * sin / 2;
		B_position.x = B_position.x + Q * cos / 2;
		B_position.y = B_position.y + Q * sin / 2;

		rx1 = A_speed.x * cos + A_speed.y * sin;
		rx2 = B_speed.x * cos + B_speed.y * sin;
		cy1 = -A_speed.x * sin + A_speed.y * cos;
		cy2 = -B_speed.x * sin + B_speed.y * cos;

		cx1 = rx2;
		cx2 = rx1;

		A_speed.x = cx1 * cos - cy1 * sin;
		A_speed.y = cx1 * sin + cy1 * cos;
		B_speed.x = cx2 * cos - cy2 * sin;
		B_speed.y = cx2 * sin + cy2 * cos;
		A_speed = A_speed.normalization();
		B_speed = B_speed.normalization();
		return true;
	}
	else
		return false;
}
bool Matching::Action_reflection_circle(FieldObject* obj, Vector2 B_position, float B_radius)
{
	Vector2& A_position = obj->position;
	Vector2& A_speed = obj->direction;
	float A_radius = obj->getRadius();

	float dx, dy, R, D, cos, sin, Q, cy1, rx1;
	dx = B_position.x - A_position.x;
	dy = B_position.y - A_position.y;
	R = A_radius + B_radius;
	D = dx * dx + dy * dy;
	if (D < R * R) {
		D = sqrt(D);
		cos = dx / D;
		sin = dy / D;
		Q = R - D;
		A_position.x = A_position.x - Q * cos;
		A_position.y = A_position.y - Q * sin;

		rx1 = A_speed.x * cos + A_speed.y * sin;
		cy1 = -A_speed.x * sin + A_speed.y * cos;

		A_speed.x = -rx1 * cos - cy1 * sin;
		A_speed.y = -rx1 * sin + cy1 * cos;
		A_speed = A_speed.normalization();
		return true;
	}
	else
		return false;
}
bool Matching::Action_reflection_circle(Player* player, Vector2 B_position, float B_radius)
{
	Vector2& A_position = player->crtList[0].position;
	Vector2& A_speed = player->crtList[0].direction;
	float A_radius = matchingData->crt_radius;

	float dx, dy, R, D, cos, sin, Q, cy1, rx1;
	dx = B_position.x - A_position.x;
	dy = B_position.y - A_position.y;
	R = A_radius + B_radius;
	D = dx * dx + dy * dy;
	if (D < R * R) {
		D = sqrt(D);
		cos = dx / D;
		sin = dy / D;
		Q = R - D;
		A_position.x = A_position.x - Q * cos;
		A_position.y = A_position.y - Q * sin;

		rx1 = A_speed.x * cos + A_speed.y * sin;
		cy1 = -A_speed.x * sin + A_speed.y * cos;

		A_speed.x = -rx1 * cos - cy1 * sin;
		A_speed.y = -rx1 * sin + cy1 * cos;
		A_speed = A_speed.normalization();
		return true;
	}
	else
		return false;
}
bool Matching::gameFinish()
{
	if (count >= matchingData->frame_rate * matchingData->playtime)
	{
		if (DataBaseManager::GetInstance()->removeMatching(this))
		{
			EndGame();
			printf("Matching finished (time over)\n");
			delete this;
		}
		return true;
	}
	if (p[0]->score >= matchingData->finish_score || p[1]->score >= matchingData->finish_score)
	{
		if (DataBaseManager::GetInstance()->removeMatching(this))
		{
			EndGame();
			printf("Matching finished (exceed fixed score)\n");
			delete this;
		}
		return true;
	}

	return false;
}
bool Matching::prepare()
{
	if(DEBUG_MATCHING)
		return false;//디버그용

	if (progress == PG_PREPARE) return true;
	else return false;
}
void Matching::ProtectDelete(User* user, bool set)
{
	if (user == p[P1]->userInfo) {
		if (set)	SetEvent(protectDelete[P1]);
		else		ResetEvent(protectDelete[P1]);
	}
	else if (user == p[P2]->userInfo)
	{
		if (set)	SetEvent(protectDelete[P2]);
		else		ResetEvent(protectDelete[P2]);
	}
}
void Matching::WaitProtectDelete()
{
	WaitForMultipleObjects(2, protectDelete, true, INFINITE);
}
void Matching::setReady(User* user)
{
	if (user == p[P1]->userInfo)
	{
		p[P1]->ready = true;
	}
	else if (user == p[P2]->userInfo)
	{
		p[P2]->ready = true;
	}

	if (p[P1]->ready && p[P2]->ready && progress == PG_PREPARE)
	{
		progress = PG_GAME;
		count = 0;

		SendPacket sp;
		sp << SERVER_GAMESTART;
		p[P1]->userInfo->sendMsg(sp);
		p[P2]->userInfo->sendMsg(sp);
		p[P1]->init();
		p[P2]->init();
	}
}