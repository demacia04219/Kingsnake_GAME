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
	int gem;				// ����
	int gemOfVictory;		// �¸��� ����(���� �ִ� ����: ����)
	int costume;			// ���� �ڽ�Ƭ ����(index)

	int matchingCount;		// �� ���� ��
	int victoryCount;		// �� ��
	int victoryStreak;		// ���� ��
	int loseStreak;			// ���� ��
	int score;				// ����
};
struct UserItemData
{
	int itemSlot[3];

	std::vector<int> items;	// ���� ������(index)
};
struct UserCharacterData
{
	int subCharacterSlot[3];
	
	int leaderCostume;		// ���� ĳ���� �ڽ�Ƭ(index)

	std::vector<int> configCostume;// ����ĳ���ͺ� ������ �ڽ�Ƭ(index)
	std::vector<int> costumes;// ���� �ڽ�Ƭ(index)
};
struct UserEmotionData
{
	int emotionSlot[3];
	std::vector<int> emotions;	// ���� �̸��(index)
};



struct UserData
{
	std::string id;
	std::string nick;

	UserBasicData basic;
	UserItemData item;
	UserCharacterData character;
	UserEmotionData emotion;

	int recvCheck;			// DB Server�κ��� ������ �޾Ҵ��� Ȯ�ο�

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
	void tempInit();	// �ӽ�
};

#endif