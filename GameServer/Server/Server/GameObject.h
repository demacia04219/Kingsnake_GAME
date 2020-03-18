#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__

#include "TimerManager.h"
#include "Global.h"
#include "Vector.h"
#include "MemoryPool.h"

class User;
class Matching;
class ActiveData;


/* �ΰ��� ��� ������Ʈ */
class GameObject
{
protected:
	User& user;
	Matching& matching;
	ActiveData* activeData;// ActiveData = ��ų, ������ ������

	int serial;	// ������ȣ

public:

	GameObject(User*, Matching*, ActiveData*);
	~GameObject();
	int getSerial() { return serial; }

	virtual void recv() = 0;
	virtual void process() = 0;
	virtual bool isActivited() = 0;
};

/* �ʵ� �� ��ġ�Ǵ� ������Ʈ */
class FieldObject : public GameObject
{
private:
	FIELDOBJECT_TYPE type;
public:
	Vector2 position;
	Vector2 direction;
	FieldObject(User* u, Matching* m, FIELDOBJECT_TYPE type, ActiveData* a) :GameObject(u, m, a), type(type) {}
	FIELDOBJECT_TYPE getType() { return type; }
	virtual float getRadius() { return 0; }// �ʿ信 ���� override. Matching::hitTest �迭 �Լ��� �� �Լ��� ȣ���Ͽ� �浹 �˻�
};




class Sword : public FieldObject // �׸��ڰ� ����ü
{
	enum {
		D_SPEED,// ����ü�ӵ� 
		D_DISTANCE,// ��Ÿ�
		D_START_TIME,// �غ�ð�
		D_HIT_RANGE,// ��Ʈ����
		D_DAMAGE,// ������
	};
public:
	float moveDistance;// �̵� �Ÿ�
	bool actived;

	Sword(User* u, Matching* m);

	void recv() override;
	void process() override;
	bool isActivited() override;
	float getRadius() override;
};
class Bubble : public FieldObject // ��� ���� ����ü
{
private:
	enum {
		D_SPEED,// ����ü�ӵ�
		D_REMOVE_TIME,// �ҰŽð�
		D_HIT_RANGE,// ��Ʈ ����
		D_DAMAGE,// ������
		D_ESCAPE_TIME,// Ż�� �ð�
	};
	void attack();		// �� ���� ó��
	void reflection();	// ������Ʈ �浹 �ݻ�
public:
	float time;	// ��� �ð�
	bool actived;
	int lock;
	Bubble(User* u, Matching* m);

	void recv() override;
	void process() override;
	bool isActivited() override;
	float getRadius() override;
};
struct Rabbit;
class RabbitBomb : public FieldObject // �����䳢 ����ü
{
	enum {
		D_DISTANCE,		// ���� �Ÿ�
		D_SPEED,		// ���� �ӵ�
		D_HIT_RANGE,	// ��Ʈ ����
		D_DAMAGE,		// ������
		D_DURATION,		// ���ӽð�
		D_BOMB_RANGE,	// ���� ����
	};
private:
	bool actived;		// ��Ƽ�� ��������
	bool move;			// �̵� ��������
	float distance;		// �̵� �Ÿ�
	bool runTime;		// ���� �� ��� �ð�
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
class Arrow : public FieldObject // ȭ������ ����ü
{
	enum {
		D_SPEED,		// ����ü �ӵ�
		D_DESTROY_TIME,	// �ҰŽð�
		D_HIT_RANGE,	// ��Ʈ ����
		D_DAMAGE,		// ������
		D_STUN_TIME,	// ���Ͻð�
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
class BlackHole : public FieldObject // ����Ʈ�ҷο� ����ü
{
	enum {
		D_DISTANCE,		// ��Ȧ �Ÿ�
		D_HIT_RANGE,	// ����
		D_DESTROY_TIME,	// �ҰŽð�
		D_DAMAGE,		// ������
		D_MAGNET_POWER,	// �ڼ�����
	};
	bool actived;
	int time;
public:
	BlackHole(User* u, Matching* m);
	void recv() override;
	void process() override;
	bool isActivited() override;
};
class MagnetArrow : public FieldObject // ��Ȥ ����ü
{
	enum {
		D_SPEED,			// ����ü �ӵ�
		D_DISTANCE,			// �̵� �Ÿ�
		D_HIT_RANGE,		// ��Ʈ����
		D_DAMAGE,			// ������
		D_MAGNET_POWER,		// ������� �ӵ�
		D_MAGNET_TIME,		// ������� �ð�
	};
	float distance;			// �̵��Ÿ�
	int allurementTime;		// ��Ȥ�� �ɸ� �ð�
	bool actived;
public:
	MagnetArrow(User* u, Matching* m);
	void recv() override;
	void process() override;
	bool isActivited() override;
	float getRadius() override;
};


#endif