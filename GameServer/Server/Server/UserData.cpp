#include "UserData.h"

void UserData::tempInit()
{
	id = "ididid";
	nick = "nicknicknick";

	basic.lv = 1;
	basic.exp = 0;
	basic.gold = 0;
	basic.gem = 0;
	basic.matchingCount = 0;
	basic.victoryCount = 0;
	basic.victoryStreak = 0;
	basic.loseStreak = 0;
	basic.gemOfVictory = 0;
	basic.costume = -1;
	basic.score = 0;

	character.leaderCostume = -1;
	character.configCostume.resize(3);

	for (int idx = 0; idx < 3; idx++) {
		item.itemSlot[idx] = idx;
		character.subCharacterSlot[idx] = idx;
		character.configCostume[idx] = -1;
		emotion.emotionSlot[idx] = -1;
	}

}