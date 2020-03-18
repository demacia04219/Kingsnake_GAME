#ifndef __MATCHING_DIALOG_H__
#define __MATCHING_DIALOG_H__

#include "Global.h"
#include "Dialog.h"
#include "resource.h"
#include "Vector.h"
#include <vector>


class Gimmick;

struct Rect {
	Vector2D leftBottom;
	Vector2D rightTop;
	Rect() {}
	Rect(Vector2D leftBottom, Vector2D rightTop) {
		this->leftBottom = leftBottom;
		this->rightTop = rightTop;
	}
	float getLeft() { return leftBottom.x; }
	float getRight() { return rightTop.x; }
	float getBottom() { return leftBottom.y; }
	float getTop() { return rightTop.y; }
};

enum PLAYER_STATE;

struct Character{
	int index;
	Vector2D position;
	float hp;
	float maxHp;
};
struct Player {
	PLAYER_STATE stat;
	float joystick;
	float speed;
	float skillGauge;
	float stamina;
	int ping;
	int crtCount;
	Vector2D direction;
	Character crt[4];
};
struct MatchingInfo{
	int serial;
	unsigned int frame;
	Player player[2];
	std::vector<Vector2D> obstacleList;
	std::vector<Gimmick*> gimmickList;

	// 기본 const 정보
	Rect _field;
	Rect _display;
	float _crt_radius;
	float _obstacle_radius;
	void initGimmickList();
};
class MatchingDialog;
struct Log {
	friend class MatchingDialog;
private:
	char logName[200];
public:
	DEBUG_PROTOCOL protocol;
	Player player;
	int index;
	unsigned int frame;
	void* data = nullptr;
	~Log() {
		if (data != nullptr) {
			delete data;
		}
	}
};


class MatchingDialog : public Dialog
{
private:
	bool drawAble = false;
	Log* selectLog1;
	Log* selectLog2;
	Gimmick* selectGimmick;
	int selectGimmickIdx;
	MatchingInfo info;
	Vector2D mouse;

	// 이벤트
	void eventMouseMove(LPARAM lParam);
	void eventTimer();
	void eventDraw();
	void eventInit();
	void eventCommand(WPARAM wParam);

	// 페인트 출력
	void draw(HDC hdc);
	void drawState(HDC hdc, int player, int x, int y);
	void drawLog(HDC hdc, Log* log);
	void drawGimmick(HDC hdc);
	void drawObstacle(HDC hdc);
	void drawRect(HDC hdc, int x1, int y1, int x2, int y2, int color);
	void drawPlayer(HDC hdc);
	void drawToolTip(HDC hdc);
	void drawGimmickInfo(HDC hdc, Vector2D pos, Gimmick& g, int idx=-1);

	// 다이얼로그 컨트롤 출력
	void showLog(int id, Log* log);

	// 유틸
	Vector2D fieldToDisplay(Vector2D pos);
	Vector2D displayToField(Vector2D pos);

public:
	Rect display = Rect(Vector2D(20, 20), Vector2D(720, 720));

	MatchingDialog(MatchingInfo info)
		: Dialog(DIALOG_MATCHING), info(info),selectLog1(nullptr), selectLog2(nullptr), selectGimmick(nullptr) {}
	BOOL MessageProcess(UINT iMessage, WPARAM wParam, LPARAM lParam) override;

	MatchingInfo* getMatchingInfo();
	void addLog(Log* log, void* data = 0, LOG_TYPE type = LOG_TYPE::NORMAL);
};

#endif