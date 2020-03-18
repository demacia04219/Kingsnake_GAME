#ifndef __USER_DATA_H__
#define __USER_DATA_H__

#include <string>
#include <vector>

#define		SIZE_NICK		30

struct UserBasicData
{
	int lv;
	int exp;
	int gold;
	int gem;				// 보석
	int gemOfVictory;		// 승리의 보석(매일 주는 보상: 보석)
	int costume;			// 메인 코스튬 설정(index)

	int matchingCount;		// 총 전투 수
	int victoryCount;		// 승 수
	int victoryStreak;		// 연승 수
	int loseStreak;			// 연패 수
	int score;				// 승점
};
struct UserItemData
{
	int itemSlot[3];

	std::vector<int> items;	// 소유 아이템(index)
};
struct UserCharacterData
{
	int subCharacterSlot[3];
	
	int leaderCostume;		// 리더 캐릭터 코스튬(index)

	std::vector<int> configCostume;// 서브캐릭터별 설정된 코스튬(index)
	std::vector<int> costumes;// 소유 코스튬(index)
};
struct UserEmotionData
{
	int emotionSlot[3];
	std::vector<int> emotions;	// 소유 이모션(index)
};



struct UserData
{
	std::string id;
	std::string nick;

	UserBasicData basic;
	UserItemData item;
	UserCharacterData character;
	UserEmotionData emotion;

	int recvCheck;			// DB Server로부터 데이터 받았는지 확인용

	UserData() : recvCheck(UD_NONE) { tempInit(); }

	enum USER_DATA{
		UD_NONE						= 0,
		UD_BASICINFO				= (1 << 0),
		UD_MATCHINFO				= (1 << 1),
		UD_CHARACTER_DACK			= (1 << 2),
		UD_ITEM_DACK				= (1 << 3),
		UD_EMOTION_DACK				= (1 << 4),
		UD_CHARACTER_INVENTORY		= (1 << 5),
		UD_ITEM_INVENTORY			= (1 << 6),
		UD_COSTUME_INVENTORY		= (1 << 7),
		UD_EMOTION_INVENTORY		= (1 << 8),

		UD_ALL						= (1 << 9) - 1,
	};

private:
	void tempInit();	// 임시
};

#endif