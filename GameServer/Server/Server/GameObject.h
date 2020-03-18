#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__

#include "TimerManager.h"
#include "Global.h"
#include "Vector.h"
#include "MemoryPool.h"

class User;
class Matching;
class ActiveData;


/* 인게임 모든 오브젝트 */
class GameObject
{
protected:
	User& user;
	Matching& matching;
	ActiveData* activeData;// ActiveData = 스킬, 아이템 데이터

	int serial;	// 고유번호

public:

	GameObject(User*, Matching*, ActiveData*);
	~GameObject();
	int getSerial() { return serial; }

	virtual void recv() = 0;
	virtual void process() = 0;
	virtual bool isActivited() = 0;
};

/* 필드 상에 배치되는 오브젝트 */
class FieldObject : public GameObject
{
private:
	FIELDOBJECT_TYPE type;
public:
	Vector2 position;
	Vector2 direction;
	FieldObject(User* u, Matching* m, FIELDOBJECT_TYPE type, ActiveData* a) :GameObject(u, m, a), type(type) {}
	FIELDOBJECT_TYPE getType() { return type; }
	virtual float getRadius() { return 0; }// 필요에 따라 override. Matching::hitTest 계열 함수도 이 함수를 호출하여 충돌 검사
};




class Sword : public FieldObject // 그림자검 투사체
{
	enum {
		D_SPEED,// 투사체속도 
		D_DISTANCE,// 사거리
		D_START_TIME,// 준비시간
		D_HIT_RANGE,// 히트범위
		D_DAMAGE,// 데미지
	};
public:
	float moveDistance;// 이동 거리
	bool actived;

	Sword(User* u, Matching* m);

	void recv() override;
	void process() override;
	bool isActivited() override;
	float getRadius() override;
};
class Bubble : public FieldObject // 방울 감옥 투사체
{
private:
	enum {
		D_SPEED,// 투사체속도
		D_REMOVE_TIME,// 소거시간
		D_HIT_RANGE,// 히트 범위
		D_DAMAGE,// 데미지
		D_ESCAPE_TIME,// 탈출 시간
	};
	void attack();		// 적 공격 처리
	void reflection();	// 오브젝트 충돌 반사
public:
	float time;	// 경과 시간
	bool actived;
	int lock;
	Bubble(User* u, Matching* m);

	void recv() override;
	void process() override;
	bool isActivited() override;
	float getRadius() override;
};
struct Rabbit;
class RabbitBomb : public FieldObject // 원폭토끼 투사체
{
	enum {
		D_DISTANCE,		// 날라갈 거리
		D_SPEED,		// 날라갈 속도
		D_HIT_RANGE,	// 히트 범위
		D_DAMAGE,		// 데미지
		D_DURATION,		// 지속시간
		D_BOMB_RANGE,	// 폭발 범위
	};
private:
	bool actived;		// 액티브 상태인지
	bool move;			// 이동 상태인지
	float distance;		// 이동 거리
	bool runTime;		// 안착 후 경과 시간
	Rabbit* skill;

public:
	RabbitBomb(User* u, Matching* m, Rabbit* r);
	void recv() override;
	void process() override;
	bool isActivited() override;
	float getRadius() override;
	void reflection();
	void explosion();
};
class Arrow : public FieldObject // 화살저격 투사체
{
	enum {
		D_SPEED,		// 투사체 속도
		D_DESTROY_TIME,	// 소거시간
		D_HIT_RANGE,	// 히트 범위
		D_DAMAGE,		// 데미지
		D_STUN_TIME,	// 스턴시간
	};
	int time;
	bool actived;
public:
	Arrow(User* u, Matching* m);
	void recv() override;
	void process() override;
	bool isActivited() override;
	float getRadius() override;
};
class BlackHole : public FieldObject // 나이트할로우 투사체
{
	enum {
		D_DISTANCE,		// 웜홀 거리
		D_HIT_RANGE,	// 범위
		D_DESTROY_TIME,	// 소거시간
		D_DAMAGE,		// 데미지
		D_MAGNET_POWER,	// 자석강도
	};
	bool actived;
	int time;
public:
	BlackHole(User* u, Matching* m);
	void recv() override;
	void process() override;
	bool isActivited() override;
};
class MagnetArrow : public FieldObject // 매혹 투사체
{
	enum {
		D_SPEED,			// 투사체 속도
		D_DISTANCE,			// 이동 거리
		D_HIT_RANGE,		// 히트범위
		D_DAMAGE,			// 데미지
		D_MAGNET_POWER,		// 끌어당기는 속도
		D_MAGNET_TIME,		// 끌어당기는 시간
	};
	float distance;			// 이동거리
	int allurementTime;		// 매혹에 걸린 시간
	bool actived;
public:
	MagnetArrow(User* u, Matching* m);
	void recv() override;
	void process() override;
	bool isActivited() override;
	float getRadius() override;
};


#endif