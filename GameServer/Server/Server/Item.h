#ifndef __ITEM_H__
#define __ITEM_H__

#include "Global.h"

class User;
class Matching;
struct ItemData;

struct Item {
protected:
	ITEM_TYPE type;
	User& user;
	Matching& matching;
	ItemData* itemData;
	int coolTime;
	int runTime;

	int getSlotIndex();

public:
	Item(User* u, Matching* m, ITEM_TYPE type);
	int getCoolFrame();
	float getCoolTime();

	virtual void recv() = 0;
	virtual void process() = 0;
	virtual bool isActived() = 0;

	static Item* createItem(ITEM_TYPE idx, User* user, Matching* matching);
};

class RabbitBomb;
struct Rabbit : public Item {
	RabbitBomb* bomb;

	Rabbit(User* u, Matching* m);
	virtual void recv() override;
	virtual void process() override;
	virtual bool isActived() override;
};
struct ArrowSnipe : public Item {
	ArrowSnipe(User* u, Matching* m);
	virtual void recv() override;
	virtual void process() override;
	virtual bool isActived() override;
};
struct Scratch : public Item {
	enum {
		D_ATTACK_DAMAGE,		// 공격데미지
		D_HIT_RANGE,			// 히트범위
		D_DURATION,				// 지속시간
		D_BLEEDING_DAMAGE,		// 출혈데미지(초)
	};
	Scratch(User* u, Matching* m);
	virtual void recv() override;
	virtual void process() override;
	virtual bool isActived() override;
};
struct NightHollow : public Item {
	NightHollow(User* u, Matching* m);
	virtual void recv() override;
	virtual void process() override;
	virtual bool isActived() override;
};
struct Curse : public Item {
	Curse(User* u, Matching* m);
	virtual void recv() override;
	virtual void process() override;
	virtual bool isActived() override;
};
struct Teleport : public Item {
	Teleport(User* u, Matching* m);
	virtual void recv() override;
	virtual void process() override;
	virtual bool isActived() override;
};
struct Allurement : public Item {// 매혹
	Allurement(User* u, Matching* m);
	virtual void recv() override;
	virtual void process() override;
	virtual bool isActived() override;
};

#endif