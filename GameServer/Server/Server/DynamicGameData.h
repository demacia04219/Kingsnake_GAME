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

/* ��ֹ� */
struct Obstacle {
	Vector2 position;
	Obstacle() {}
	Obstacle(float x, float y) { position.x = x, position.y = y; }
};

/* ��� */
struct Gimmick{
	MAP_OBJECT_TYPE index;
	Vector2 position;
	Matching* matching;

	Gimmick() {}
	Gimmick(float x, float y) :position(x, y) {}
	virtual bool run() { return false; }// Matching���� ������ �ֱ� ���� �Լ�(��ȯ�� false : ������ �ֱ� ���� ��󿡼� ����)
	virtual void recv(User& user) {}// Ŭ��κ��� ��� �������� ���� �� ����

	virtual void packing(SendPacket& packet);
	static Gimmick* createGimmick(MAP_OBJECT_TYPE type, Gimmick* copyTarget = nullptr);
};

struct Temple : public Gimmick {
	enum GemType{A=2,B,C};	// ĳ���� �Ϸ� ȿ��. A(���ݷ�), B(�ִ�ü��), C(ü������ȸ��)
	float maxGauge;
	float gauge;
	bool casting;
	int cooltime;
	bool allIntoTemple;
	GemType gemType;
	Player* firstPlayer; // Temple�� ���� �÷��̾�
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
	enum STAT{NORMAL, ACTIVITY, DIE};// ���� ����, ������ ��ġ �� Ȱ��ȭ ����, ���� �� ��Ȱ��ȭ ����

	STAT stat;
	int timer;

	Mandora():stat(NORMAL), timer(0){}
	bool explosion(Player* p);
	bool run() override;
	void recv(User& user) override;
};
struct Bush : public Gimmick // ��Ǯ
{
	bool run() override { return false; }
	void recv(User& user) override {};
};
struct Marsh : public Gimmick // ��
{
	std::vector<Player*> player;
	bool run() override;
	void recv(User& user) override;
};
struct Castle : public Gimmick // ��
{
	void recv(User& user) override;
};
struct Gem : public Gimmick // ����
{
	static std::map<int, unsigned int> startFrame;
	bool activity;
	Gem() :activity(true) {}
	bool run() override;
	void recv(User& user) override;
	bool acquire(Player* p);
};
struct BlessingTree : public Gimmick // ������
{
	bool activity;
	int type;					// Ʈ�� ����(0:����(ü��), 1:�Ķ�(��ų������), 2:���(���׹̳�))
	int deathCount;
	BlessingTree() :type(0), activity(true), deathCount(0) {};
	bool run() override;
	void recv(User& user) override;
};
struct Cat : public Gimmick // �����
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
