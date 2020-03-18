#include "DynamicGameData.h"
#include "Global.h"
#include "Matching.h"
#include "DataBase.h"
#include "DebugClientManager.h"
#include "TCPClient.h"
#include "CSVReader.h"

void MatchingData::init()
{
	string matching_csv, map_csv;

	switch (type) {
	case MATCHING_TYPE::BASIC:
		matching_csv = CSV_MATCHING1;
		map_csv = CSV_MAP1;
		break;
	}

	CSVReader l;
	cout << "MatchingData " << type << endl;
	if (l.read(matching_csv)) {
		frame_rate = l.getInt(1, 2);
		map_size = Rect(Vector2(l.getFloat(1, 3), l.getFloat(1, 4)),
			Vector2(l.getFloat(1, 5), l.getFloat(1, 6)));
		map_block_width = l.getInt(1, 7);
		map_block_height = l.getInt(1, 8);
		p1_startPosition = Vector2(l.getFloat(1, 9), l.getFloat(1, 10));
		p2_startPosition = Vector2(l.getFloat(1, 11), l.getFloat(1, 12));
		p1_startDirection = Vector2(l.getFloat(1, 13), l.getFloat(1, 14)).normalization();
		p2_startDirection = Vector2(l.getFloat(1, 15), l.getFloat(1, 16)).normalization();
		crt_radius = l.getFloat(1, 17);
		obstacle_radius = l.getFloat(1, 18);
		playtime = l.getInt(1, 19);
		finish_score = l.getInt(1, 20);
		cout << "\tCompleted to load \"" << matching_csv << "\"" << endl;
	}
	else {
		cout << "\tFailed to load \"" << matching_csv << "\"" << endl;
		valid = false;
	}

	std::map<int, Vector2> wayPoint;
	if (l.read(map_csv)) {
		for (int y = 12; y < 12 + map_block_height; y++) {
			for (int x = 10; x < 10 + map_block_width; x++) {
				float obj = l.getFloat(x, y);
				// 장애물
				if (obj > 0 && obj <= 4) {
					Obstacle* o = new Obstacle(x - 10, y - 12);
					o->position = blockToField(o->position);
					obstacle.push_back(o);
				}
				// 기믹
				if (obj >= 5 && obj <= 13) {
					Gimmick* g = Gimmick::createGimmick((MAP_OBJECT_TYPE)(int)obj);
					g->position = Vector2(x - 10, y - 12);
					g->position = blockToField(g->position);
					gimmick.push_back(g);
				}
				// 몽블랑 웨이포인트
				if (obj >= 14 && obj < 15) {
					Vector2 pos(x - 10, y - 12);
					pos = blockToField(pos);
					wayPoint[(int)(obj * 10) % 10] = pos;
				}
			}
		}
		catWayPoint = std::vector<Vector2>(wayPoint.size());
		for (auto d : wayPoint) {
			catWayPoint[d.first - 1] = d.second;
		}
		cout << "\tCompleted to load \"" << map_csv << "\"" << endl;
	}
	else {
		cout << "\tFailed to load \"" << map_csv << "\"" << endl;
		valid = false;
	}
}
void MatchingData::dispose()
{
	while (obstacle.size() > 0) {
		delete obstacle[0];
		obstacle.erase(obstacle.begin() + 0);
	}
	while (gimmick.size() > 0) {
		delete gimmick[0];
		gimmick.erase(gimmick.begin() + 0);
	}
	catWayPoint.clear();
}

Vector2 MatchingData::blockToField(Vector2 blockPos)
{
	float width = abs(map_size.getRight() - map_size.getLeft()) / map_block_width;
	float height = abs(map_size.getTop() - map_size.getBottom()) / map_block_height;

	blockPos.y = map_block_height - blockPos.y - 1;

	Vector2 pos;
	pos.x = map_size.getLeft();
	pos.y = map_size.getBottom();

	pos.x += width / 2 + blockPos.x * width;
	pos.y += height / 2 + blockPos.y * height;

	return pos;
}
StaticGameData::StaticGameData() {
	load();
}
void StaticGameData::load()
{
	CSVReader reader;
	bool canRun = true;

	// 기믹 csv
	if (reader.read(CSV_GIMMICK)) {
		gimmick.clear();
		for (int y = 2; y < reader.yCount(); y++) {
			bool find = gimmick.find(reader.getInt(1, y)) != gimmick.end();
			GimmickData _g;
			GimmickData& g = (find ? gimmick[reader.getInt(1,y)] : (_g = GimmickData()));

			g.index = (MAP_OBJECT_TYPE)reader.getInt(1, y);
			g.radius = reader.getFloat(2, y);
			for (int n = 0; n < 5; n++)
				g.data[n] = reader.getFloat(3 + n, y);

			if(!find)
				gimmick[g.index] = g;
		}
		cout << "Completed to load \"" << CSV_GIMMICK << "\"" << endl;
	}
	else {
		cout << "Failed to load \"" << CSV_GIMMICK << "\"" << endl;
		canRun = false;
	}

	// 스킬
	if (reader.read(CSV_SKILL)) {
		skill.clear();
		for (int y = 3; y < reader.yCount(); y++) {
			bool find = skill.find(reader.getInt(0, y)) != skill.end();
			SkillData _s;
			SkillData& s = (find ? skill[reader.getInt(0, y)] : (_s = SkillData()));

			s.index = reader.getInt(0, y);
			s.name = reader.getString(1, y);
			s.coolTime = reader.getFloat(3, y);
			s.runTime = reader.getFloat(4, y);
			s.gaugeCost = reader.getFloat(5, y);
			for (int n = 0; n < 7; n++)
				s.data[n] = reader.getFloat(6 + n, y);

			if (!find)
				skill[s.index] = s;
		}
		cout << "Completed to load \"" << CSV_SKILL << "\"" << endl;
	}
	else {
		cout << "Failed to load \"" << CSV_SKILL << "\"" << endl;
		canRun = false;
	}

	// 아이템
	if(reader.read(CSV_ITEM)){
		item.clear();
		for (int y = 3; y < reader.yCount(); y++) {
			bool find = item.find(reader.getInt(0, y)) != item.end();
			ItemData _s;
			ItemData& s = (find ? item[reader.getInt(0, y)] : (_s = ItemData()));

			s.index = reader.getInt(0, y);
			s.name = reader.getString(1, y);
			s.coolTime = reader.getFloat(3, y);
			s.runTime = reader.getFloat(4, y);
			s.gaugeCost = reader.getFloat(5, y);
			for (int n = 0; n < 7; n++)
				s.data[n] = reader.getFloat(6 + n, y);

			if (!find)
				item[s.index] = s;
		}
		cout << "Completed to load \"" << CSV_ITEM << "\"" << endl;
	}
	else {
		cout << "Failed to load \"" << CSV_ITEM << "\"" << endl;
		canRun = false;
	}

	// 리더
	if (reader.read(CSV_LEADER)) {
		leader.maxHP = reader.getFloat(1, 2);
		leader.maxStamina = reader.getFloat(1, 3);
		leader.speed = reader.getFloat(1, 4);
		leader.damage = reader.getFloat(1, 5);
		leader.decrementStaminaAttack = reader.getFloat(1, 6);
		leader.decrementStaminaDefense = reader.getFloat(1, 7);
		leader.decrementStaminaCasting = reader.getFloat(1, 8);
		leader.respawnDelay = reader.getFloat(1, 9);
		leader.recoveryStamina = reader.getFloat(1, 10);
		leader.maxSkillGauge = reader.getFloat(1, 11);
		leader.increaseSpeed = reader.getFloat(1, 12);
		leader.recoverySkillGauge = reader.getFloat(1, 13);
		leader.attackRecovery = reader.getFloat(1, 14);
		cout << "Completed to load \"" << CSV_LEADER << "\"" << endl;
	}
	else {
		cout << "Failed to load \"" << CSV_LEADER << "\"" << endl;
		canRun = false;
	}

	// 서브 캐릭터
	if(reader.read(CSV_SUB)){
		sub.clear();
		for (int y = 5; y < reader.yCount(); y++) {
			bool find = sub.find(reader.getInt(0, y)) != sub.end();
			SubCharacterData _s;
			SubCharacterData& s = (find ? sub[reader.getInt(0, y)] : (_s = SubCharacterData()));
			
			s.index = reader.getInt(0, y);
			s.name = reader.getString(1, y);
			s.type = reader.getInt(2, y);
			s.skill = reader.getInt(3, y);
			s.extraStamina = reader.getFloat(4, y);
			s.extraSpeed = reader.getFloat(5, y);
			s.maxHP = reader.getFloat(6, y);
			s.damage = reader.getFloat(7, y);
		
			if (!find)
				sub[s.index] = s;
		}
		cout << "Completed to load \"" << CSV_SUB << "\"" << endl;
	}
	else {
		cout << "Failed to load \"" << CSV_SUB << "\"" << endl;
		canRun = false;
	}

	// 캐릭터 전투유형 별 추가 스텟
	if(reader.read(CSV_CRT_EFFECT)){
		crtEffect.clear();
		for (int y = 2; y < reader.yCount(); y++) {
			bool find = crtEffect.find(reader.getInt(0, y)) != crtEffect.end();
			CharacterEffectData _eff;
			CharacterEffectData& eff = (find ? crtEffect[reader.getInt(0, y)] : (_eff = CharacterEffectData()));
			
			eff.index = reader.getInt(0, y);
			eff.type = reader.getInt(1, y);
			eff.count = reader.getInt(2, y);
			for (int n = 0; n < 2; n++)
				eff.data[n] = reader.getFloat(3 + n, y);

			if (!find)
				crtEffect[eff.index] = eff;
		}
		cout << "Completed to load \"" << CSV_CRT_EFFECT << "\"" << endl;
	}
	else {
		cout << "Failed to load \"" << CSV_CRT_EFFECT << "\"" << endl;
		canRun = false;
	}

	// 패시브
	if (reader.read(CSV_PASSIVE)) {
		passive.clear();
		for (int y = 2; y < reader.yCount(); y++) {
			bool find = passive.find(reader.getInt(0, y)) != passive.end();
			PassiveData _s;
			PassiveData& p = (find ? passive[reader.getInt(0, y)] : (_s = PassiveData()));
			
			p.index = reader.getInt(0, y);
			p.type = reader.getInt(2, y);
			p.count = reader.getInt(3, y);
			for (int n = 0; n < 2; n++)
				p.data[n] = reader.getFloat(4 + n, y);

			if (!find)
				passive[p.index] = p;
		}
		cout << "Completed to load \"" << CSV_PASSIVE << "\"" << endl;
	}
	else {
		cout << "Failed to load \"" << CSV_PASSIVE << "\"" << endl;
		canRun = false;
	}

	// 코스튬
	if (reader.read(CSV_COSTUME)) {
		passive.clear();
		for (int y = 2; y < reader.yCount(); y++) {
			bool find = costume.find(reader.getInt(0, y)) != costume.end();
			CostumeData _s;
			CostumeData& p = (find ? costume[reader.getInt(0, y)] : (_s = CostumeData()));

			p.index = reader.getInt(0, y);
			p.subCharacter = reader.getInt(1, y);
			p.price = reader.getInt(2, y);

			if (!find)
				costume[p.index] = p;
		}
		cout << "Completed to load \"" << CSV_COSTUME << "\"" << endl;
	}
	else {
		cout << "Failed to load \"" << CSV_COSTUME << "\"" << endl;
		canRun = false;
	}

	// 매칭
	if (matching.find(MATCHING_TYPE::BASIC) != matching.end()) {
		matching[MATCHING_TYPE::BASIC]->dispose();
		matching[MATCHING_TYPE::BASIC]->init();
	}
	else
		matching[MATCHING_TYPE::BASIC] = new MatchingData(MATCHING_TYPE::BASIC);

	// 유효한 매칭데이터인지 검사
	for (auto m : matching) {
		if (m.second->valid == false)
		{
			canRun = false; break;
		}
	}
	
	cout << endl;

	// 최종 csv loading 결과
	if (!canRun) {
		cout << "Failed to run Game Server" << endl;
		system("pause");
		exit(0);
	}
}