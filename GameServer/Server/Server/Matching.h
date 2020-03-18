#ifndef __ROOM_H__
#define __ROOM_H__

#include "GlobalProtocol.h"
#include "Player.h"
#include "TimerManager.h"
#include "DynamicGameData.h"
#include <vector>
#include <map>

class Rect;
class Gimmick;
struct GameObject;
struct Skill;
class FieldObject;

class Matching : public TimerProcess
{
	enum PROGRESS{PG_PREPARE, PG_GAME};// ��Ī ���� ����
private:
	MATCHING_TYPE type;
	MatchingData* matchingData;
	Temple* temple;
	Cat* cat;
	Player* p[2];
	HANDLE protectDelete[2];
	CRITICAL_SECTION cs;
	unsigned int count;	// ��� ������
	int serial;
	double tickTime;	// ���� �����Ǵ� ƽ �и���
	HANDLE canDelete;
	std::vector<Gimmick*> gimmicks;
	std::vector<Gimmick*> runGimmicks;	// ������ �ð��� �ֱ�� ��� run�Լ��� ȣ���� ��� ���
	std::map<int, GameObject*> gameObjects;
	PROGRESS progress;

	bool prepare();			// ��Ī ���� �� �غ�. true�� �غ� ����
	bool gameFinish();		//
	void processFrame();	// process ���� �۾�(������ ó��)
	void lerp();			// ���� �۾�
	bool isLerpAvailable(); // ������ �������� üũ

public:
	Matching(User*, User*, MATCHING_TYPE type);
	~Matching();

	void StartGame();
	void EndGame(User* disconnectUser = nullptr);
	Player* getPlayer1();
	Player* getPlayer2();
	Player* getPlayerOf(User* user);
	Player* getEnemyOf(User* user);
	Player* getPlayer(int index);
	Temple& getTemple();
	Cat& getCat();
	int getSerial();
	unsigned int getCurrentFrame();

	int getGimmicksCount();
	Gimmick& getGimmick(int index);
	MatchingData& getMatchingData();
	int getIndexOf(Gimmick& g);
	void addRuns(Gimmick* g);
	void addRuns(GameObject* g);
	GameObject* getGameObject(int serial);
	void setReady(User* user);

	void process() override;

	bool Action_reflection_player(Player*, Player*);											// ĳ���� ���ο� ������ �浹
	bool Action_reflection_circle(Player*, Vector2 B_position, float B_radius);				// ĳ���� - �� �浹 üũ �� �ݻ� 
	bool Action_reflection_circle(FieldObject* obj, Vector2 B_position, float B_radius);
	bool Action_reflection_box(Player* player, float radius, Rect box);	// ĳ���� - �簢�� �浹 üũ �� �ݻ�
	bool Action_reflection_Wall(Player*);														// ĳ���� - �ʵ� �ܰ� �� �浹 üũ
	bool Action_reflection_Wall(FieldObject* obj);

	bool Action_hitTest_circle(Player*, Vector2 B_position, float B_radius);
	bool Action_hitTest_circle(Vector2 A_position, float A_radius, Vector2 B_position, float B_radius);					// ĳ���� - �� �浹 üũ
	bool Action_hitTest_box(Vector2& position, float radius, Rect box);		// ĳ���� - �簢�� �浹 üũ
	bool Action_hitTest_wall(Player*);															// ĳ���� - �ʵ� �ܰ� �� �浹 üũ
	
	void EnterCS();
	void LeaveCS();
	void ProtectDelete(User* user, bool set);
	void WaitProtectDelete();
};

#endif
