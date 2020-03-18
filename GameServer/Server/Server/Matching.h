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
	enum PROGRESS{PG_PREPARE, PG_GAME};// 매칭 진행 상태
private:
	MATCHING_TYPE type;
	MatchingData* matchingData;
	Temple* temple;
	Cat* cat;
	Player* p[2];
	HANDLE protectDelete[2];
	CRITICAL_SECTION cs;
	unsigned int count;	// 경과 프레임
	int serial;
	double tickTime;	// 실제 구동되는 틱 밀리초
	HANDLE canDelete;
	std::vector<Gimmick*> gimmicks;
	std::vector<Gimmick*> runGimmicks;	// 프레임 시간을 주기로 계속 run함수를 호출할 기믹 대상
	std::map<int, GameObject*> gameObjects;
	PROGRESS progress;

	bool prepare();			// 매칭 시작 전 준비. true면 준비 상태
	bool gameFinish();		//
	void processFrame();	// process 내부 작업(프레임 처리)
	void lerp();			// 보간 작업
	bool isLerpAvailable(); // 보간이 가능한지 체크

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

	bool Action_reflection_player(Player*, Player*);											// 캐릭터 선두와 선두의 충돌
	bool Action_reflection_circle(Player*, Vector2 B_position, float B_radius);				// 캐릭터 - 원 충돌 체크 및 반사 
	bool Action_reflection_circle(FieldObject* obj, Vector2 B_position, float B_radius);
	bool Action_reflection_box(Player* player, float radius, Rect box);	// 캐릭터 - 사각형 충돌 체크 및 반사
	bool Action_reflection_Wall(Player*);														// 캐릭터 - 필드 외곽 벽 충돌 체크
	bool Action_reflection_Wall(FieldObject* obj);

	bool Action_hitTest_circle(Player*, Vector2 B_position, float B_radius);
	bool Action_hitTest_circle(Vector2 A_position, float A_radius, Vector2 B_position, float B_radius);					// 캐릭터 - 원 충돌 체크
	bool Action_hitTest_box(Vector2& position, float radius, Rect box);		// 캐릭터 - 사각형 충돌 체크
	bool Action_hitTest_wall(Player*);															// 캐릭터 - 필드 외곽 벽 충돌 체크
	
	void EnterCS();
	void LeaveCS();
	void ProtectDelete(User* user, bool set);
	void WaitProtectDelete();
};

#endif
