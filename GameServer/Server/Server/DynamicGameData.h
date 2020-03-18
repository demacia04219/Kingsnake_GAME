#ifndef __DYNAMIC_GAME_DATA_H__
#define __DYNAMIC_GAME_DATA_H__

#include "StaticGameData.h"
#include "Packet.h"
#include <map>
#include "MemoryPool.h"

#define SKILLCONTAINER_COUNT 3

class User;
class Matching;
class Player;

/* 장애물 */
struct Obstacle {
	Vector2 position;
	Obstacle() {}
	Obstacle(float x, float y) { position.x = x, position.y = y; }
};

/* 기믹 */
struct Gimmick{
	MAP_OBJECT_TYPE index;
	Vector2 position;
	Matching* matching;

	Gimmick() {}
	Gimmick(float x, float y) :position(x, y) {}
	virtual bool run() { return false; }// Matching에서 프레임 주기 실행 함수(반환값 false : 프레임 주기 실행 대상에서 제명)
	virtual void recv(User& user) {}// 클라로부터 기믹 프로토콜 받을 때 실행

	virtual void packing(SendPacket& packet);
	static Gimmick* createGimmick(MAP_OBJECT_TYPE type, Gimmick* copyTarget = nullptr);
};

struct Temple : public Gimmick {
	enum GemType{A=2,B,C};	// 캐스팅 완료 효과. A(공격력), B(최대체력), C(체력지속회복)
	float maxGauge;
	float gauge;
	bool casting;
	int cooltime;
	bool allIntoTemple;
	GemType gemType;
	Player* firstPlayer; // Temple에 닿은 플레이어
	Temple() :
		gauge(0), maxGauge(100), firstPlayer(nullptr),
		casting(false), cooltime(0), allIntoTemple(false), gemType(A) { }
	Temple(float x, float y) : Gimmick(x, y),
		gauge(0), maxGauge(100), firstPlayer(nullptr),
		casting(false), cooltime(0), allIntoTemple(false), gemType(A) { }

	bool run() override;
	virtual void packing(SendPacket& packet) override;

	bool enter(Player& p, bool _bool);
	bool tryToCasting(Player& p, Player& enemy, bool _bool, bool autoLogSend = true);
	bool isEnter(Player* p);
	bool isCasting(Player* p);
	bool isCanCasting(Player* p);
};
struct Mandora :public Gimmick {
	enum STAT{NORMAL, ACTIVITY, DIE};// 보통 상태, 만도라 터치 후 활성화 상태, 폭발 후 비활성화 상태

	STAT stat;
	int timer;

	Mandora():stat(NORMAL), timer(0){}
	bool explosion(Player* p);
	bool run() override;
	void recv(User& user) override;
};
struct Bush : public Gimmick // 수풀
{
	bool run() override { return false; }
	void recv(User& user) override {};
};
struct Marsh : public Gimmick // 늪
{
	std::vector<Player*> player;
	bool run() override;
	void recv(User& user) override;
};
struct Castle : public Gimmick // 성
{
	void recv(User& user) override;
};
struct Gem : public Gimmick // 보석
{
	static std::map<int, unsigned int> startFrame;
	bool activity;
	Gem() :activity(true) {}
	bool run() override;
	void recv(User& user) override;
	bool acquire(Player* p);
};
struct BlessingTree : public Gimmick // 생명나무
{
	bool activity;
	int type;					// 트리 종류(0:빨강(체력), 1:파랑(스킬게이지), 2:노랑(스테미나))
	int deathCount;
	BlessingTree() :type(0), activity(true), deathCount(0) {};
	bool run() override;
	void recv(User& user) override;
};
struct Cat : public Gimmick // 고양이
{
	enum STAT{STAND, MOVE, COME_BACK};
	
	STAT stat;
	Vector2 direction;
	Vector2 home;
	int wayPointIdx;
	int totalCount;
	int p1Count;
	int gemCount;
	int time;

	Cat() : stat(STAND), totalCount(0), p1Count(0), wayPointIdx(0), gemCount(0), time(0) {}
	bool run() override;
	void recv(User& user) override;
	void recv(User& user, Gem* gem);
};


#endif                                                                                                                 
