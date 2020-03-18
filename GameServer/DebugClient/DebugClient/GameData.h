#pragma once

#include "Vector.h"

enum MAP_OBJECT_TYPE
{
	IDLE = 0,
	OBSTACLE_TREE_SMALL = 1,
	OBSTACLE_TREE_MEDIUM,
	OBSTACLE_TREE_LARGE,
	OBSTACLE_ROCK,

	GIMMICK_BUSH,
	GIMMICK_MANDRA,
	GIMMICK_MARSH,		// ´Ë
	GIMMICK_CASTLE_BLUE,
	GIMMICK_CASTLE_RED,
	GIMMICK_TEMPLE,
	GIMMICK_GEM,
	GIMMICK_BLESSINGTREE,
	GIMMICK_FAIRY,
	NOT_USE_1,
	NOT_USE_2,
};
struct Gimmick
{
	MAP_OBJECT_TYPE type;
	Vector2D position;
	float radius;
	bool hitTest(Vector2D vec);
	static const char* gimmickName[];
};
struct Temple : public Gimmick {
	float maxGauge;
	float gauge;
	bool casting;
	int cooltime;
	int firstPlayer;
};

class TCPClient;

class GameObject {
public:
	static Gimmick* UnPacking(TCPClient& client, MAP_OBJECT_TYPE type);
	static Gimmick* Copy(Gimmick* gimmick);
};