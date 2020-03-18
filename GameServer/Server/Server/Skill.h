#ifndef __SKILL_H__
#define __SKILL_H__

#include "Global.h"

class User;
class Matching;
class SkillData;

struct Skill {
protected:
	SKILL_TYPE type;
	User& user;
	Matching& matching;
	SkillData* skillData;
	int coolTime;
	int runTime;		// 시전 시간(스킬 사용 후 경과 시간. 단위 : 프레임)

	int getSlotIndex();

public:
	Skill(User* u, Matching* m, SKILL_TYPE type);
	int getCoolFrame();
	float getCoolTime();
	
	virtual void recv() = 0;
	virtual void process() = 0;
	virtual bool isActived() = 0;

	static Skill* createSkill(SKILL_TYPE type, User* user, Matching* matching);
};

struct BubbleTrap :public Skill {

	BubbleTrap(User* u, Matching* m);
	~BubbleTrap();

	virtual void recv() override;
	virtual void process() override;
	virtual bool isActived() override;
};
struct ShadowSword : public Skill {

	ShadowSword(User* u, Matching* m);
	~ShadowSword();

	virtual void recv() override;
	virtual void process() override;
	virtual bool isActived() override;
};
struct WindPush : public Skill {

	WindPush(User* u, Matching* m);
	~WindPush();

	virtual void recv() override;
	virtual void process() override;
	virtual bool isActived() override;
};

#endif
