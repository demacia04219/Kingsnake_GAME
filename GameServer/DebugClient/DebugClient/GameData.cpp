#include "TCPClient.h"
#include "GameData.h"
const char* Gimmick::gimmickName[] = {
	   "idle","나무(소)","나무(중)","나무(대)","바위",
	   "수풀","만도라","늪","푸른성","붉은성","제단","보석",
	   "생명나무","요정","",""
};
Gimmick* GameObject::UnPacking(TCPClient& client, MAP_OBJECT_TYPE type)
{
	Gimmick* gimmick;
	switch (type) {
	case MAP_OBJECT_TYPE::GIMMICK_TEMPLE:
	{
		Temple* tmp = new Temple();
		client >> tmp->type >> tmp->position >> tmp->radius;
		client >> tmp->maxGauge >> tmp->gauge >> tmp->casting >> tmp->cooltime >> tmp->firstPlayer;
		gimmick = tmp;
	}
		break;
	default: 
		gimmick = new Gimmick();
		client >> gimmick->type >> gimmick->position >> gimmick->radius;
		break;
	}
	return gimmick;
}
Gimmick* GameObject::Copy(Gimmick* gimmick) {
	Gimmick* g;
	switch (gimmick->type) {
	case MAP_OBJECT_TYPE::GIMMICK_TEMPLE:
		{Temple* tmp = new Temple();	*tmp = *((Temple*)gimmick);	g = tmp;}
		break;
	default:
		{g = new Gimmick();		*g = *gimmick; }
		break;
	}
	return g;
}
bool Gimmick::hitTest(Vector2D vec)
{
	return ((position - vec).size() <= radius);
}