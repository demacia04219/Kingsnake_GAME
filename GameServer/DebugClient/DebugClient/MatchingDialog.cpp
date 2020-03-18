#include "NetworkManager.h"
#include "MatchingDialog.h"
#include "ProjectManager.h"
#include "resource.h"
#include "GameData.h"

BOOL MatchingDialog::MessageProcess(UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	if (NetworkManager::GetInstance()->isConnect() == false)
	{
		return 0;
	}
	switch (iMessage)
	{
	case WM_INITDIALOG:
		eventInit();
		return 0;
	case WM_PAINT:
		eventDraw();
		return 0;
	case WM_MOUSEMOVE:
		eventMouseMove(lParam);
		return 0;
	case WM_TIMER:
		eventTimer();
		return 0;
	case WM_COMMAND:
		eventCommand(wParam);
		return 0;
	}
	return 0;
}
Vector2D MatchingDialog::fieldToDisplay(Vector2D pos)
{
	float scaleX = (display.getRight() - display.getLeft()) / (info._field.rightTop.x - info._field.leftBottom.x);
	float scaleY = (display.getTop() - display.getBottom()) / (info._field.rightTop.y - info._field.leftBottom.y);

	pos = pos + info._field.leftBottom * (-1);
	pos.y = -pos.y;

	pos.x *= scaleX;
	pos.y *= scaleY;

	pos.x += display.getLeft();
	pos.y += display.getTop();

	return pos;
}
Vector2D MatchingDialog::displayToField(Vector2D pos)
{
	float scaleX = (display.getRight() - display.getLeft()) / (info._field.rightTop.x - info._field.leftBottom.x);
	float scaleY = (display.getTop() - display.getBottom()) / (info._field.rightTop.y - info._field.leftBottom.y);

	pos.x -= display.getLeft();
	pos.y -= display.getTop();

	pos.x /= scaleX;
	pos.y /= scaleY;

	pos.y = -pos.y;

	pos = pos - info._field.leftBottom * (-1);

	return pos;
}
void MatchingDialog::showLog(int id, Log* log)
{
	char msg[100];
	sprintf(msg, "Frame %d\n", log->frame);
	text_setString(id, msg);
	text_addString(id, log->logName);

	sprintf(msg, "ping: %d ms\n", log->player.ping);
	text_addString(id, msg);

	switch (log->player.stat) {
	case PLAYER_STATE::MOVE:	strcpy(msg, "상태 : 이동\n"); break;
	case PLAYER_STATE::ATTACK:	strcpy(msg, "상태 : 공격\n"); break;
	case PLAYER_STATE::GUARD:	strcpy(msg, "상태 : 방어\n"); break;
	case PLAYER_STATE::DEATH:	strcpy(msg, "상태 : 죽음\n"); break;
	}
	text_addString(id, msg);

	if (log->player.joystick != -999)
		sprintf(msg, "조이스틱: %.0f˚\n", log->player.joystick);
	else
		sprintf(msg, "조이스틱: X\n");
	text_addString(id, msg);

	sprintf(msg, "스피드: %.3f\n", log->player.speed);
	text_addString(id, msg);

	if (log->player.stat != PLAYER_STATE::DEATH) {
		for (int i = 0; i < log->player.crtCount; i++) {
			//sprintf(msg, "캐릭%d HP:%.1f\n", i + 1, log->player.crt[i].hp);
			sprintf(msg, "캐릭%d HP(%.1f / %.1f)\n", log->player.crt[i].index, log->player.crt[i].hp, log->player.crt[i].maxHp);
			text_addString(id, msg);
		}
	}
}
void MatchingDialog::drawLog(HDC hdc, Log* log)
{
	if (log == nullptr)
		return;

	int color;
	if (log->index == 0)	color = 0x0000ff;
	else						color = 0xff0000;

	Player& player = log->player;

	HPEN hPen, oldPen;
	HBRUSH hBrush, oldBrush;
	hBrush = CreateSolidBrush(0xeeeeee);
	oldBrush = (HBRUSH)SelectObject(hdc, hBrush);
	switch (log->protocol) {
	case DEBUG_PROTOCOL::SERVER_HIT_ATTACK:
		hPen = CreatePen(0, 2, 0x999999);
		oldPen = (HPEN)SelectObject(hdc, hPen);
		Player& enemy = *((Player*)log->data);
		Vector2D p = fieldToDisplay(enemy.crt[0].position);
		Vector2D dir = fieldToDisplay(enemy.crt[0].position + enemy.direction * 3);
		MoveToEx(hdc, p.x, p.y, 0);
		LineTo(hdc, dir.x, dir.y);
		for (int j = 0; j < enemy.crtCount; j++)
		{
			Vector2D scale = Vector2D(info._crt_radius, info._crt_radius);
			Vector2D p1 = fieldToDisplay(enemy.crt[j].position - scale);
			Vector2D p2 = fieldToDisplay(enemy.crt[j].position + scale);
			Ellipse(hdc, p1.x, p1.y, p2.x, p2.y);
		}
		SelectObject(hdc, oldPen);
		DeleteObject(hPen);
		break;
	}


	hPen = CreatePen(0, 2, color);
	oldPen = (HPEN)SelectObject(hdc, hPen);
	Vector2D p = fieldToDisplay(player.crt[0].position);
	Vector2D dir = fieldToDisplay(player.crt[0].position + player.direction * 3);
	MoveToEx(hdc, p.x, p.y, 0);
	LineTo(hdc, dir.x, dir.y);
	for (int j = 0; j < player.crtCount; j++)
	{
		Vector2D scale = Vector2D(info._crt_radius, info._crt_radius);
		Vector2D p1 = fieldToDisplay(player.crt[j].position - scale);
		Vector2D p2 = fieldToDisplay(player.crt[j].position + scale);
		Ellipse(hdc, p1.x, p1.y, p2.x, p2.y);
	}
	SelectObject(hdc, oldPen);
	DeleteObject(hPen);
	SelectObject(hdc, oldBrush);
	DeleteObject(hBrush);
}
void MatchingDialog::draw(HDC hdc)
{
	// 필드 바탕
	drawRect(hdc, display.getLeft(), display.getBottom(), display.getRight(), display.getTop(), 0xeeeeee);

	// 장애물, 기믹
	drawObstacle(hdc);
	drawGimmick(hdc);

	// 로그
	drawLog(hdc, selectLog1);
	drawLog(hdc, selectLog2);

	// 플레이어
	drawPlayer(hdc);

	// 현재 플레이어 스텟
	{
		char frame[30];
		sprintf(frame, "Frame %d", info.frame);
		//text_setString(MATCHING_TXT1, frame);
		Rectangle(hdc, display.getRight() + 10, display.getTop(), display.getRight() + 200, display.getBottom());
		TextOut(hdc, display.getRight() + 20, display.getBottom() + 5, "현재 플레이어 정보", 18);

		TextOut(hdc, display.getRight() + 20, display.getBottom() + 5 + 20, frame, strlen(frame));

		drawState(hdc, 0, display.getRight() + 20, display.getBottom() + 60);
		drawState(hdc, 1, display.getRight() + 20, (display.getBottom() + display.getTop()) / 2 + 10);
	}

	// 기믹 정보 ToolTip
	drawToolTip(hdc);
}
MatchingInfo* MatchingDialog::getMatchingInfo()
{
	return &info;
}
void MatchingDialog::addLog(Log* log, void* data, LOG_TYPE type)
{
	if (checkbox_getCheck(MATCHING_CHECK1)) {
		return;
	}
	if (checkbox_getCheck(MATCHING_CHECK2) &&
		type == LOG_TYPE::COLLISION_WALL_OBSTACLE) {
		return;
	}
	if (checkbox_getCheck(MATCHING_CHECK3) &&
		type == LOG_TYPE::TEMPLE) {
		return;
	}

	char frame[30] = { 0 };
	char logNmae[200] = { 0 };
	char listName[230] = { 0 };
	sprintf(frame, "[%d]  ", log->frame);

	switch (log->protocol)
	{
	case DEBUG_PROTOCOL::SERVER_CHANGE_STATE:
		switch (log->player.stat) {
		case PLAYER_STATE::MOVE:	sprintf(logNmae, "%s[이동] 상태로 변경", logNmae); break;
		case PLAYER_STATE::ATTACK:	sprintf(logNmae, "%s[공격] 상태로 변경", logNmae); break;
		case PLAYER_STATE::GUARD:	sprintf(logNmae, "%s[방어] 상태로 변경", logNmae); break;
		case PLAYER_STATE::DEATH:	sprintf(logNmae, "%s[죽음] 상태로 변경", logNmae); break;
		}
		break;
	case DEBUG_PROTOCOL::SERVER_COLLISION_WALL:
		sprintf(logNmae, "%s%s", logNmae, data);
		delete data;
		break;
	case DEBUG_PROTOCOL::SERVER_HIT_ATTACK:
		sprintf(logNmae, "%s%s", logNmae, data);
		delete data;
		break;
	}

	int id = (log->index == 0 ? MATCHING_LIST1 : MATCHING_LIST2);

	bool last = false;
	if (listbox_getSelect(id) == listbox_getCount(id) - 1) {
		last = true;
	}

	sprintf(listName, "%s%s", frame, logNmae);
	listbox_addItem(id, listName, (long long)log);
	sprintf(log->logName, "%s\n", logNmae);

	if (last) {
		listbox_setSelect(id, listbox_getCount(id) - 1);
		Log* log = (Log*)(unsigned long long)listbox_getData(id, listbox_getSelect(id));
		if (log->index == 0)	selectLog1 = log;
		else					selectLog2 = log;

		int id = (log->index == 0 ? MATCHING_TXT2 : MATCHING_TXT3);
		showLog(id, log);
	}

	// 200개 항목을 초과하지 못하게.
	if (listbox_getCount(id) > 200) {
		Log* log = (Log*)(unsigned long long)listbox_getData(id, 0);
		delete log;
		listbox_removeItem(id, 0);
	}
}

void MatchingDialog::drawState(HDC hdc, int player_index, int x, int y)
{
	char name[300];
	int ypos = 0;

	Player player = info.player[player_index];
	sprintf(name, "Player%d", player_index + 1);
	TextOut(hdc, x, y, name, strlen(name));

	sprintf(name, "ping: %dms", player.ping);
	TextOut(hdc, x, (y += 20), name, strlen(name));

	switch (player.stat) {
	case PLAYER_STATE::MOVE:	sprintf(name, "상태 : 이동"); break;
	case PLAYER_STATE::ATTACK:	sprintf(name, "상태 : 공격"); break;
	case PLAYER_STATE::GUARD:	sprintf(name, "상태 : 방어"); break;
	case PLAYER_STATE::DEATH:	sprintf(name, "상태 : 죽음"); break;
	}

	TextOut(hdc, x, (y += 20), name, strlen(name));

	if (player.joystick != -999)
		sprintf(name, "조이스틱: %.0f˚", player.joystick);
	else
		sprintf(name, "조이스틱: X");
	TextOut(hdc, x, (y += 20), name, strlen(name));
	sprintf(name, "스피드: %.3f", player.speed);
	TextOut(hdc, x, (y += 20), name, strlen(name));
	sprintf(name, "스테미나 : %.1f", player.stamina);
	TextOut(hdc, x, (y += 20), name, strlen(name));
	sprintf(name, "스킬게이지 : %.1f", player.skillGauge);
	TextOut(hdc, x, (y += 20), name, strlen(name));

	if (player.stat != PLAYER_STATE::DEATH) {
		for (int i = 0; i < player.crtCount; i++) {
			sprintf(name, "캐릭%d HP (%.1f / %.1f)", player.crt[i].index, player.crt[i].hp, player.crt[i].maxHp);
			TextOut(hdc, x, (y += 20), name, strlen(name));
		}
	}
}

void MatchingDialog::drawRect(HDC hdc, int x1, int y1, int x2, int y2, int color)
{
	HBRUSH hBrush = CreateSolidBrush(color);
	HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, hBrush);
	Rectangle(hdc, x1, y1, x2, y2);
	SelectObject(hdc, oldBrush);
	DeleteObject(hBrush);
}
void MatchingInfo::initGimmickList()
{
	while (gimmickList.size() > 0) {
		delete gimmickList[0];
		gimmickList.erase(gimmickList.begin() + 0);
	}
}
void MatchingDialog::drawGimmick(HDC hdc)
{
	int count = info.gimmickList.size();

	for (int i = 0; i < count; i++)
	{
		Gimmick& g = *(info.gimmickList[i]);
		Vector2D pos = fieldToDisplay(g.position);

		// 외형
		{
			HBRUSH hBrush = CreateSolidBrush(0xffff00);
			HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, hBrush);
			HPEN hPen = 0, oldPen = 0;
			if (selectGimmick == &g) {
				hPen = CreatePen(0, 3, 0xff0000);
				oldPen = (HPEN)SelectObject(hdc, hPen);
			} else {
				hPen = CreatePen(0, 1, 0xaaaa00);
				oldPen = (HPEN)SelectObject(hdc, hPen);
			}

			switch (g.type) {
			case MAP_OBJECT_TYPE::GIMMICK_BUSH:
			case MAP_OBJECT_TYPE::GIMMICK_MARSH:
			{
				Vector2D scale = Vector2D(g.radius, g.radius);
				Vector2D p1 = fieldToDisplay(g.position - scale);
				Vector2D p2 = fieldToDisplay(g.position + scale);
				Rectangle(hdc, p1.x, p1.y, p2.x, p2.y);
			}
			break;
			default:
			{
				Vector2D scale = Vector2D(g.radius, g.radius);
				Vector2D p1 = fieldToDisplay(g.position - scale);
				Vector2D p2 = fieldToDisplay(g.position + scale);
				Ellipse(hdc, p1.x, p1.y, p2.x, p2.y);
			}
			break;
			}

			SelectObject(hdc, oldPen);
			DeleteObject(hPen);
			SelectObject(hdc, oldBrush);
			DeleteObject(hBrush);
		}
		// 세부
		{
			if (checkbox_getCheck(MATCHING_CHECK4)) {
				Vector2D p = g.position;
				p.y -= g.radius + 1;
				p = fieldToDisplay(p);
				drawGimmickInfo(hdc, p, g, i);
			}
		}
	}// for
}
void MatchingDialog::eventMouseMove(LPARAM lParam)
{
	mouse.x = LOWORD(lParam);
	mouse.y = HIWORD(lParam);
}
void MatchingDialog::eventTimer()
{
	Vector2D mousePosInField = displayToField(mouse);

	selectGimmick = nullptr;

	int count = info.gimmickList.size();
	for (int i = count - 1; i >= 0; i--) {
		if (info.gimmickList[i]->hitTest(mousePosInField))
		{
			selectGimmick = info.gimmickList[i];
			selectGimmickIdx = i;
			break;
		}
	}
}
void MatchingDialog::eventDraw()
{
	RECT rect;
	PAINTSTRUCT ps;
	GetClientRect(hWnd, &rect);
	HDC hdc = BeginPaint(hWnd, &ps);
	HDC MemDC = CreateCompatibleDC(hdc);
	HBITMAP MyBit = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
	HBITMAP OldBit = (HBITMAP)SelectObject(MemDC, MyBit);
	drawRect(MemDC, rect.left, rect.top, rect.right, rect.bottom, 0xaaaaaa);

	if (drawAble)
		draw(MemDC);
	drawAble = true;

	BitBlt(hdc, 0, 0, rect.right, rect.bottom, MemDC, 0, 0, SRCCOPY);
	SelectObject(MemDC, OldBit);
	DeleteObject(MyBit);
	DeleteDC(MemDC);

	EndPaint(hWnd, &ps);
}
void MatchingDialog::eventInit()
{
	char name[100];
	Matching* m = NetworkManager::GetInstance()->getMatchingOf(info.serial);
	if (m == nullptr) {
		return;
	}
	sprintf(name, "[빨강] P1: %s", NetworkManager::GetInstance()->getIP(m->player1));
	text_setString(MATCHING_TXT4, name);
	sprintf(name, "[파랑] P2: %s", NetworkManager::GetInstance()->getIP(m->player2));
	text_setString(MATCHING_TXT5, name);

	SetTimer(hWnd, 0, 0, 0);
}
void MatchingDialog::eventCommand(WPARAM wParam)
{
	switch (LOWORD(wParam)) {
	case IDCANCEL:
		while (listbox_getCount(MATCHING_LIST1) > 0) {
			Log* log = (Log*)(unsigned long long)listbox_getData(MATCHING_LIST1, 0);
			delete log;
			listbox_removeItem(MATCHING_LIST1, 0);
		}
		while (listbox_getCount(MATCHING_LIST2) > 0) {
			Log* log = (Log*)(unsigned long long)listbox_getData(MATCHING_LIST2, 0);
			delete log;
			listbox_removeItem(MATCHING_LIST2, 0);
		}
		close();
		NetworkManager::GetInstance()->sendLeaveMatching(info.serial);
		ProjectManager::GetInstance()->removeMatchingDialog(this);
		delete this;
		return;
	case MATCHING_LIST1:
		if (HIWORD(wParam) == LBN_SELCHANGE) {
			Log* log = (Log*)(unsigned long long)listbox_getData(MATCHING_LIST1, listbox_getSelect(MATCHING_LIST1));
			selectLog1 = log;
			showLog(MATCHING_TXT2, log);
		}
		return;
	case MATCHING_LIST2:
		if (HIWORD(wParam) == LBN_SELCHANGE) {
			Log* log = (Log*)(unsigned long long)listbox_getData(MATCHING_LIST2, listbox_getSelect(MATCHING_LIST2));
			selectLog2 = log;
			showLog(MATCHING_TXT3, log);
		}
		return;
	}
}
void MatchingDialog::drawObstacle(HDC hdc)
{
	HPEN hPen = CreatePen(0, 1, 0x00aa00);
	HPEN oldPen = (HPEN)SelectObject(hdc, hPen);
	HBRUSH hBrush = CreateSolidBrush(0x00ff00);
	HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, hBrush);
	int count = info.obstacleList.size();
	for (int i = 0; i < count; i++) {
		Vector2D scale = Vector2D(info._obstacle_radius, info._obstacle_radius);
		Vector2D p1 = fieldToDisplay(info.obstacleList[i] - scale);
		Vector2D p2 = fieldToDisplay(info.obstacleList[i] + scale);
		Ellipse(hdc, p1.x, p1.y, p2.x, p2.y);
	}
	SelectObject(hdc, oldBrush);
	DeleteObject(hBrush);
	SelectObject(hdc, oldPen);
	DeleteObject(hPen);
}
void MatchingDialog::drawPlayer(HDC hdc)
{
	int color[2] = { 0x0000ff, 0xff0000 };
	for (int i = 0; i < 2; i++) {
		Player& player = info.player[i];

		if (player.stat == PLAYER_STATE::DEATH) {
			continue;
		}

		HBRUSH hBrush = CreateSolidBrush(color[i]);
		HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, hBrush);

		Vector2D p = fieldToDisplay(player.crt[0].position);
		Vector2D dir = fieldToDisplay(player.crt[0].position + player.direction * 3);
		MoveToEx(hdc, p.x, p.y, 0);
		LineTo(hdc, dir.x, dir.y);

		for (int j = 0; j < player.crtCount; j++)
		{
			Vector2D scale = Vector2D(info._crt_radius, info._crt_radius);
			Vector2D p1 = fieldToDisplay(player.crt[j].position - scale);
			Vector2D p2 = fieldToDisplay(player.crt[j].position + scale);
			Ellipse(hdc, p1.x, p1.y, p2.x, p2.y);
		}
		SelectObject(hdc, oldBrush);
		DeleteObject(hBrush);

		for (int j = 0; j < player.crtCount; j++)
		{
			Vector2D p = fieldToDisplay(player.crt[j].position);
			p.y -= 16;
			drawRect(hdc, p.x - 20, p.y - 3, p.x + 10, p.y + 3, 0x555555);
			drawRect(hdc, p.x - 20, p.y - 3, p.x - 10 + (player.crt[j].hp / player.crt[j].maxHp) * 20, p.y + 3, 0x0000ff);
		}
	}
}
void MatchingDialog::drawToolTip(HDC hdc)
{
	if (selectGimmick == nullptr) return;

	Gimmick& g = *selectGimmick;
	Vector2D pos = mouse;
	pos.x += 15; pos.y += 15;

	drawGimmickInfo(hdc, pos, g, selectGimmickIdx);
}
void MatchingDialog::drawGimmickInfo(HDC hdc, Vector2D pos, Gimmick& g, int idx)
{
	RECT rt = { pos.x, pos.y };
	switch (g.type) {
	case MAP_OBJECT_TYPE::GIMMICK_TEMPLE:
	{
		char msg[100];
		Temple& t = (Temple&)g;

		sprintf(msg, "* %s *\n", Gimmick::gimmickName[g.type]);
		sprintf(msg, "%sindex: %d\n", msg, idx);
		sprintf(msg, "%s좌표: (%.2f, %.2f)\n", msg, g.position.x, g.position.y);
		sprintf(msg, "%s게이지: %.0f\n", msg, t.gauge);
		sprintf(msg, "%s쿨타임: %.0f초\n", msg, ceil(((float)t.cooltime) / 40.0));

		if (t.casting)	sprintf(msg, "%s캐스팅: true\n", msg);
		else			sprintf(msg, "%s캐스팅: false\n", msg);

		if (t.firstPlayer == -1)		sprintf(msg, "%s점유: X\n", msg);
		else if (t.firstPlayer == 0)	sprintf(msg, "%s점유: Player1(빨강)\n", msg);
		else if (t.firstPlayer == 1)	sprintf(msg, "%s점유: Player2(파랑)", msg);

		DrawText(hdc, msg, -1, &rt, DT_LEFT | DT_NOCLIP);
	}
	break;
	default:
	{
		if (g.type > 20 || g.type < 0)	return;
		char msg[100];
		sprintf(msg, "* %s *\n",Gimmick::gimmickName[g.type]);
		sprintf(msg, "%sindex: %d\n", msg, idx);
		sprintf(msg, "%s좌표: (%.2f, %.2f)\n", msg, g.position.x, g.position.y);
		DrawText(hdc, msg, -1, &rt, DT_LEFT | DT_NOCLIP);
	}
		break;
	}
}