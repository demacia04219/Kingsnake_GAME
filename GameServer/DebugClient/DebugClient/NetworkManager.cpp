#include "NetworkManager.h"
#include "TCPClient.h"
#include "ProjectManager.h"
#include "MainDialog.h"
#include "MatchingDialog.h"
#include "GameData.h"
#include "DialogManager.h"
#include "MainManager.h"

NetworkManager* NetworkManager::Instance = nullptr;

NetworkManager::NetworkManager()
{
	//client = *(new TCPClient(SERVER_PORT, SERVER_IP));
	connected = false;
	thread = 0;
	serial = 0;
}
NetworkManager::~NetworkManager()
{
	client.stop();
	delete (&client);

	int size = clientList.size();
	for (int i = 0; i < size; i++)
	{
		delete clientList[i];
	}

	size = matchingList.size();
	for (int i = 0; i < size; i++)
	{
		delete matchingList[i];
	}
}

NetworkManager* NetworkManager::CreateInstance()
{
	if (Instance == nullptr)
	{
		Instance = new NetworkManager();
	}
	return Instance;
}
NetworkManager* NetworkManager::GetInstance()
{
	return Instance;
}
void NetworkManager::DestroyInstance()
{
	if (Instance != nullptr)
	{
		delete Instance;
		Instance = nullptr;
	}
}

bool NetworkManager::connect()
{
	if (client.start(SERVER_PORT, SERVER_IP))
	{
		connected = true;
		thread = CreateThread(0, 0, NetworkManager::RecvProcess, 0, 0, 0);


		int password[] = PASSWORD_DEBUG;
		LICENCE_PROTOCOL p = LICENCE_PROTOCOL::CLIENT_PASSWORD;

		SendPacket sp;
		sp.addData(&p, sizeof(p));
		sp.addData(password, sizeof(password));
		client.sendMsg(sp);

		return true;
	}
	else
		return false;
}
int NetworkManager::getSerial()
{
	return serial;
}
bool NetworkManager::isConnect()
{
	return connected;
}
DWORD WINAPI NetworkManager::RecvProcess(void* arg)
{
	ProjectManager::GetInstance()->waitRecv();
	NetworkManager* manager = NetworkManager::GetInstance();
	TCPClient& client = manager->client;

	while (true)
	{
		if (client.recvMsg() == false)
		{
			manager->connected = false;
			MessageBox(ProjectManager::GetInstance()->getMainHWND(), "서버와 연결이 끊어졌습니다.", "Message", MB_OK);
			ProjectManager::GetInstance()->getMainDialog()->close();
			break;
		}

		MainManager::GetInstance()->EnterCS();

		DEBUG_PROTOCOL p;
		client >> p;
		switch (p)
		{
		case (DEBUG_PROTOCOL)LICENCE_PROTOCOL::SERVER_RESULT:
			manager->recvLicenceResult();
			break;
		case DEBUG_PROTOCOL::SERVER_USERLIST:
			manager->recvUserList();
			break;
		case DEBUG_PROTOCOL::SERVER_CONNECT_USER:
			manager->recvConnectUser();
			break;
		case DEBUG_PROTOCOL::SERVER_DISCONNECT_USER:
			manager->recvDisconnectUser();
			break;
		case DEBUG_PROTOCOL::SERVER_MATCHINGLIST:
			manager->recvMatchingList();
			break;
		case DEBUG_PROTOCOL::SERVER_CREATE_MATCHING:
			manager->recvCreateMatching();
			break;
		case DEBUG_PROTOCOL::SERVER_DESTROY_MATCHING:
			manager->recvDestroyMatching();
			break;
		case DEBUG_PROTOCOL::SERVER_ENTER_MATCHING:
			manager->recvEnterMatching();
			break;
		case DEBUG_PROTOCOL::SERVER_MATCHING_FRAME:
			manager->recvMatchingFrame();
			break;
		case DEBUG_PROTOCOL::SERVER_INFO_OBSTACLE:
			manager->recvInfoObstacle();
			break;
		case DEBUG_PROTOCOL::SERVER_CHANGE_STATE:
			manager->recvChangeState();
			break;
		case DEBUG_PROTOCOL::SERVER_COLLISION_WALL:
		case DEBUG_PROTOCOL::SERVER_COLLISION_OBSTACLE:
		case DEBUG_PROTOCOL::SERVER_EVENT_TEMPLE:
			manager->recvEvent(p);
			break;
		case DEBUG_PROTOCOL::SERVER_HIT_ATTACK:
			manager->recvHitAttack();
			break;
		case DEBUG_PROTOCOL::SERVER_DEBUG_PING:
			manager->recvPing();
			break;
		case DEBUG_PROTOCOL::SERVER_INFO_GIMMICK:
			manager->recvInfoGimmick();
			break;
		case DEBUG_PROTOCOL::SERVER_CHANGE_TEMPLE:
			manager->recvChangeTemple();
			break;
		}
		MainManager::GetInstance()->LeaveCS();
	}
	return 0;
}
void NetworkManager::recvLicenceResult()
{
	bool b;
	client >> b;
	if (b) {
		SendPacket sp;
		sp << DEBUG_PROTOCOL::CLIENT_USERLIST;
		client.sendMsg(sp);
		sp.initBuffer();
		sp << DEBUG_PROTOCOL::CLIENT_MATCHINGLIST;
		client.sendMsg(sp);
	}
}
void NetworkManager::recvMatchingList()
{
	int count;
	client >> count;
	for (int i = 0; i < count; i++)
	{
		Matching* m = new Matching();
		client >> m->serial >> m->player1 >> m->player2;
		matchingList.push_back(m);
	}
	PostMessage(ProjectManager::GetInstance()->getMainHWND(), MESSAGE::MATCHINGLIST, 0, 0);
}
void NetworkManager::recvUserList()
{
	int count;

	client >> serial >> count;
	for (int i = 0; i < count; i++)
	{
		Client* c = new Client();
		client >> c->ip >> c->serial >> c->state;
		clientList.push_back(c);
	}
	PostMessage(ProjectManager::GetInstance()->getMainHWND(), MESSAGE::USERLIST, 0, 0);
}
void NetworkManager::recvConnectUser()
{
	int serial;
	char ip[16];
	USER_STATE stat;
	client >> ip >> serial >> stat;

	int count = clientList.size();
	for (int i = 0; i < count; i++)
	{
		if (clientList[i]->serial == serial)
		{
			clientList[i]->state = stat;
			PostMessage(ProjectManager::GetInstance()->getMainHWND(), MESSAGE::CONNECT, (WPARAM)clientList[i], 0);
			return;
		}
	}
	if (stat != USER_STATE::LICENCE) return;
	Client* c = new Client();
	strcpy(c->ip, ip);
	c->serial = serial;
	c->state = stat;
	clientList.push_back(c);
	PostMessage(ProjectManager::GetInstance()->getMainHWND(), MESSAGE::CONNECT, (WPARAM)c, 0);
}
void NetworkManager::recvDisconnectUser()
{
	int serial;
	client >> serial;
	int count = clientList.size();
	for (int i = 0; i < count; i++)
	{
		if (clientList[i]->serial == serial)
		{
			Client* c = clientList[i];
			clientList.erase(clientList.begin() + i);
			PostMessage(ProjectManager::GetInstance()->getMainHWND(), MESSAGE::DISCONNECT, (WPARAM)c, 0);
			break;
		}
	}
}
void NetworkManager::recvCreateMatching()
{
	Matching* m = new Matching();
	client >> m->serial >> m->player1 >> m->player2;

	matchingList.push_back(m);
	PostMessage(ProjectManager::GetInstance()->getMainHWND(), MESSAGE::CREATE_MATCHING, (WPARAM)m, 0);
}
void NetworkManager::recvDestroyMatching()
{
	int serial;
	client >> serial;

	int count = matchingList.size();
	for (int i = 0; i < count; i++) {
		if (matchingList[i]->serial == serial)
		{
			Matching* m = matchingList[i];
			matchingList.erase(matchingList.begin() + i);
			PostMessage(ProjectManager::GetInstance()->getMainHWND(), MESSAGE::DESTROY_MATCHING, (WPARAM)m, 0);
			break;
		}
	}
	ProjectManager* manager = ProjectManager::GetInstance();
	count = manager->getMatchingDialogCount();
	for (int i = 0; i < count; i++) {
		MatchingInfo* info = manager->getMatchingDialog(i)->getMatchingInfo();
		if (info->serial == serial)
		{
			PostMessage(manager->getMatchingDialog(i)->getHWND(), WM_COMMAND, MAKELONG(IDCANCEL, 0), 0);
			break;
		}
	}
}
void NetworkManager::recvEnterMatching()
{
	MatchingInfo info;
	client >> info.serial;
	client >> info._field.leftBottom >> info._field.rightTop >> info._crt_radius >> info._obstacle_radius;

	for (int i = 0; i < 2; i++) {
		info.player[i].ping = 0;
		client >> info.player[i].stat >> info.player[i].joystick;
		client >> info.player[i].speed >> info.player[i].crtCount;
		for (int j = 0; j < 4; j++) {
			Character& crt = info.player[i].crt[j];
			client >> crt.index >> crt.position >> crt.maxHp >> crt.hp;
		}
	}

	MatchingDialog* dialog = new MatchingDialog(info);
	ProjectManager::GetInstance()->addMatchingDialog(dialog);
	dialog->openOnNewThread();
}
void NetworkManager::recvMatchingFrame()
{
	int serial;
	float gauge;
	unsigned int frame;
	client >> serial >> gauge >> frame;

	ProjectManager* manager = ProjectManager::GetInstance();
	int count = manager->getMatchingDialogCount();
	for (int i = 0; i < count; i++) {
		MatchingInfo* info = manager->getMatchingDialog(i)->getMatchingInfo();
		if (info->serial == serial)
		{
			info->frame = frame;
			int cnt = info->gimmickList.size();
			for (int j = 0; j < cnt; j++)
				if (info->gimmickList[j]->type == MAP_OBJECT_TYPE::GIMMICK_TEMPLE)
					((Temple*)info->gimmickList[j])->gauge = gauge;

			for (int j = 0; j < 2; j++) {
				Player& player = info->player[j];
				client >> player.direction >> player.joystick >> player.stamina >> player.skillGauge;
				client >> player.crtCount;
				for (int k = 0; k < player.crtCount; k++) {
					client >> player.crt[k].position;
				}
			}
			Rect rect = manager->getMatchingDialog(i)->display;
			RECT rt;
			rt.left = rect.getLeft();
			rt.right = rect.getRight() + 200;
			rt.top = rect.getTop();
			rt.bottom = rect.getBottom();
			InvalidateRect(manager->getMatchingDialog(i)->getHWND(), &rt, 0);
			break;
		}
	}
}
void NetworkManager::recvInfoObstacle()
{
	int serial;
	client >> serial;

	ProjectManager* manager = ProjectManager::GetInstance();
	int count = manager->getMatchingDialogCount();
	for (int i = 0; i < count; i++) {
		MatchingInfo* info = manager->getMatchingDialog(i)->getMatchingInfo();
		if (info->serial == serial)
		{
			int count;
			Vector2D pos;
			client >> count;
			for (int j = 0; j < count; j++) {
				client >> pos;
				info->obstacleList.push_back(pos);
			}
			break;
		}
	}
}
void NetworkManager::recvChangeState()
{
	int serial, index;
	float speed;
	PLAYER_STATE stat;
	client >> serial >> index >> stat >> speed;

	ProjectManager* manager = ProjectManager::GetInstance();
	int count = manager->getMatchingDialogCount();
	for (int i = 0; i < count; i++) {
		MatchingInfo* info = manager->getMatchingDialog(i)->getMatchingInfo();
		if (info->serial == serial)
		{
			if (info->player[index].stat == PLAYER_STATE::DEATH) {
				info->player[index].crtCount = 4;
				for (int j = 0; j < 4; j++) {
					info->player[index].crt[j].index = j + 1;
					info->player[index].crt[j].hp = info->player[index].crt[j].maxHp;
				}
			}
			info->player[index].stat = stat;
			info->player[index].speed = speed;

			Log* log = new Log();
			log->index = index;
			log->player = info->player[index];
			log->frame = info->frame;
			log->protocol = DEBUG_PROTOCOL::SERVER_CHANGE_STATE;
			manager->getMatchingDialog(i)->addLog(log);
			break;
		}
	}
}
void NetworkManager::recvEvent(DEBUG_PROTOCOL p)
{
	int serial, index;
	char msg[200];
	client >> serial >> index >> msg;

	ProjectManager* manager = ProjectManager::GetInstance();
	int count = manager->getMatchingDialogCount();
	for (int i = 0; i < count; i++) {
		MatchingInfo* info = manager->getMatchingDialog(i)->getMatchingInfo();
		if (info->serial == serial)
		{
			Log* log = new Log();
			log->index = index;
			log->player = info->player[index];
			log->frame = info->frame;
			log->protocol = DEBUG_PROTOCOL::SERVER_COLLISION_WALL;

			char* data = new char[strlen(msg) + 1];
			strcpy(data, msg);

			switch (p) {
			case DEBUG_PROTOCOL::SERVER_COLLISION_WALL:
			case DEBUG_PROTOCOL::SERVER_COLLISION_OBSTACLE:
				manager->getMatchingDialog(i)->addLog(log, data, LOG_TYPE::COLLISION_WALL_OBSTACLE);
				break;
			case DEBUG_PROTOCOL::SERVER_EVENT_TEMPLE:
				manager->getMatchingDialog(i)->addLog(log, data, LOG_TYPE::TEMPLE);
				break;
			}
			break;
		}
	}
}
void NetworkManager::recvHitAttack()
{
	int serial, index;
	bool hit, attack;
	client >> serial >> hit >> attack >> index;

	ProjectManager* manager = ProjectManager::GetInstance();
	int count = manager->getMatchingDialogCount();
	for (int i = 0; i < count; i++) {
		MatchingInfo* info = manager->getMatchingDialog(i)->getMatchingInfo();
		if (info->serial == serial)
		{
			int count;
			int enemy_index = (index == 0 ? 1 : 0);
			Player* p = new Player(info->player[enemy_index]);

			client >> count;
			for (int j = 0; j < count; j++) {
				client >> info->player[enemy_index].crt[j].index;
				client >> info->player[enemy_index].crt[j].hp;
			}

			Log* log = new Log();
			log->index = index;
			log->player = info->player[index];
			log->frame = info->frame;
			log->protocol = DEBUG_PROTOCOL::SERVER_HIT_ATTACK;

			log->data = p;

			char* data = new char[100];
			sprintf(data, "적 공격 / 충돌(%d), 공격(%d)", hit, attack);

			manager->getMatchingDialog(i)->addLog(log, data);
			break;
		}
	}
}
int NetworkManager::getClientCount()
{
	return clientList.size();
}
int NetworkManager::getMatchingCount()
{
	return matchingList.size();
}
Client* NetworkManager::getClient(int index)
{
	return clientList[index];
}
Matching* NetworkManager::getMatchingOf(int serial)
{
	int count = matchingList.size();
	for (int i = 0; i < count; i++)
		if (matchingList[i]->serial == serial)
			return matchingList[i];
	return nullptr;
}
Matching* NetworkManager::getMatching(int index)
{
	return matchingList[index];
}
void NetworkManager::sendEnterMatching(int serial)
{
	SendPacket sp;
	sp << DEBUG_PROTOCOL::CLIENT_ENTER_MATCHING << serial;
	client.sendMsg(sp);
}
void NetworkManager::sendLeaveMatching(int serial)
{
	SendPacket sp;
	sp << DEBUG_PROTOCOL::CLIENT_LEAVE_MATCHING << serial;
	client.sendMsg(sp);
}
const char* NetworkManager::getIP(int clientSerial)
{
	int count = clientList.size();
	for (int i = 0; i < count; i++)
	{
		if (clientList[i]->serial == clientSerial)
		{
			return clientList[i]->ip;
		}
	}
}
void NetworkManager::recvPing()
{
	int serial, player, ping;
	client >> serial >> player >> ping;

	ProjectManager* manager = ProjectManager::GetInstance();
	int count = manager->getMatchingDialogCount();
	for (int i = 0; i < count; i++) {
		MatchingInfo* info = manager->getMatchingDialog(i)->getMatchingInfo();
		if (info->serial == serial)
		{
			info->player[player].ping = ping;
			break;
		}
	}
}
void NetworkManager::recvInfoGimmick()
{
	int serial, count;
	vector<Gimmick*> gimmicks;
	client >> serial >> count;
	for (int i = 0; i < count; i++)
	{
		MAP_OBJECT_TYPE type;
		client >> type;

		Gimmick* g = GameObject::UnPacking(client, type);
		gimmicks.push_back(g);
	}

	ProjectManager* manager = ProjectManager::GetInstance();
	int count2 = manager->getMatchingDialogCount();
	for (int j = 0; j < count2; j++) {
		MatchingInfo* info = manager->getMatchingDialog(j)->getMatchingInfo();
		if (info->serial == serial)
		{
			info->initGimmickList();
			int count3 = gimmicks.size();
			for (int k = 0; k < count3; k++) {
				Gimmick* g = GameObject::Copy(gimmicks[k]);
				info->gimmickList.push_back(g);
			}
			break;
		}
	}
	while (gimmicks.size() > 0) {
		delete gimmicks[0];
		gimmicks.erase(gimmicks.begin());
	}
}
void NetworkManager::recvChangeTemple()
{
	int serial;
	Gimmick* gimmick;

	client >> serial;
	gimmick = GameObject::UnPacking(client, MAP_OBJECT_TYPE::GIMMICK_TEMPLE);

	ProjectManager* manager = ProjectManager::GetInstance();
	int count = manager->getMatchingDialogCount();
	for (int i = 0; i < count; i++) {
		MatchingInfo* info = manager->getMatchingDialog(i)->getMatchingInfo();
		if (info->serial == serial)
		{
			int count2 = info->gimmickList.size();
			for (int j = 0; j < count2; j++) {
				if (info->gimmickList[j]->type == gimmick->type) {
					//*(info->gimmickList[j]) = *gimmick;
					delete info->gimmickList[j];
					info->gimmickList[j] = GameObject::Copy(gimmick);
					delete gimmick;
					break;
				}
			}
			break;
		}
	}
}