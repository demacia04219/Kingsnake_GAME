#ifndef __GAMEMANAGER_H__
#define __GAMEMANAGER_H__

#include <list>
#include "Matching.h"

class User;

class MatchingManager
{
	// ===============================
	// Singleton Sector
	// ===============================
	//
private:
	static MatchingManager* Instance;

	MatchingManager();
	~MatchingManager();
public:
	static void CreateInstance();
	static MatchingManager* GetInstance();
	static void DestroyInstance();

	// ===============================
	// GameManager Sector
	// ===============================
	//
private:
	void recvJoystick(User*);
	void recvCollisionWall(User*);

	void recvAttackHit(User*);
	void recvCollisionObstacle(User*);
	void recvCollisionGimmick(User*);
	void recvCollisionCat(User*);

	void recvPong(User*);

	void recvActiveMove(User*);
	void recvActiveAttack(User*);
	void recvActiveGuard(User*);
	void recvTempleEnter(User*);
	void recvTempleCasting(User*);

	void recvSkill(User*);
	void recvItemSkill(User*);
	void recvGameObject(User*);
	void recvReady(User* user);
	void recvCancelMatching(User* user);

public:
	Matching * matching(User* user);

	void recv(User*);
	void disconnect(User*);
};

#endif
