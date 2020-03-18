#ifndef PACKET_H
#define PACKET_H

#include <vector>
#include <string>

class Packet;
using SendPacket = Packet;
using RecvPacket = Packet;

class Packets;
using SendPackets = Packets;
using RecvPackets = Packets;

class IOCPClient;

// ���� ��Ŷ�� �� ���� send�ϱ� ����
class Packets
{
	friend class Packet;
	friend class IOCPClient;
private:
	std::vector<Packet*> packets;
	int size; // ��ü ũ��

public:
	Packets();
	Packets(const Packets& p);

	int memcpyWithEncrypt(void* canvas); // EncryptManager::encoding() ����
	int memcpyWithEncrypt(void* canvas, int start, int size); // start ��ġ���� size��ŭ ����. EncryptManager::encoding() ����
	int getTotalSize();
	void init(bool tryToDelete=false);

	Packets& operator =(const Packets& p);
	Packets& operator +=(Packet& p);
	Packets& operator =(Packet& p);
	Packets& operator +(Packet& p);
};

// ��ŷ�� �����ִ�
class Packet
{
private:
	using string = std::string;
	char* ptr;
	char* buffer;
	int bufferSize;		// ���� ũ��
	int* dataSize;		// ��ŷ�� ũ��

public:
	Packet(int bufferSize = 10);
	Packet(const Packet& p);
	~Packet();
	Packet& operator = (const Packet& p);
	Packets operator + (Packet& p);

	int getPackingSize();	// ��ŷ�� ������
	int getTotalSize();		// ��ŷ + ��ü ũ�� ������. ��ǻ� ��ŷ�� ������ + 4
	int getBufferSize();	// �Ҵ�� ���� ũ��
	void setBufferSize(int size);
	void initPointer();
	void initBuffer();
	char* getPackingBuffer();
	char* getTotalBuffer();

	bool canGetData(int size);
	void addData(void* data, int size);	// ���� ũ�� �ʰ� �� �ڵ� Ȯ��
	void getData(void* data, int size);




	static int wstrlen(const wchar_t* _source);
	template<class T>
	Packet& operator<<(T data) { addData(&data, sizeof(data));	return *this; }
	template<class T>
	Packet& operator<<(T* data) { addData(data, sizeof(*data));	return *this; }
	template<>
	Packet& operator<<(const char* data) {
		int size = strlen(data) + 1;
		addData(&size, sizeof(size));
		addData((void*)data, size);
		return *this;
	}
	template<>
	Packet& operator<<(const wchar_t* data) {
		int size = (wstrlen(data) + 1) * 2;
		addData(&size, sizeof(size));
		addData((void*)data, size);
		return *this;
	}
	template<>
	Packet& operator<<(string data) {
		int size = data.size() + 1;
		addData(&size, sizeof(size));
		addData((void*)data.data(), size);
		return *this;
	}
	template<>
	Packet& operator<<(char* data) {
		int size = strlen(data) + 1;
		addData(&size, sizeof(size));
		addData((void*)data, size);
		return *this;
	}
	template<>
	Packet& operator<<(wchar_t* data) {
		int size = (wstrlen(data) + 1) * 2;
		addData(&size, sizeof(size));
		addData((void*)data, size);
		return *this;
	}
	template<class T>
	Packet& operator>>(T& data) { getData(&data, sizeof(data));	return *this; }
	template<class T>
	Packet& operator>>(T* data) { getData(data, sizeof(*data)); return *this; }
	template<>
	Packet& operator>>(char* data) {
		int size;
		getData(&size, sizeof(size));
		getData(data, size);
		return *this;
	}
	template<>
	Packet& operator>>(wchar_t* data) {
		int size;
		getData(&size, sizeof(size));
		getData(data, size);
		return *this;
	}
	template<>
	Packet& operator>>(string& data) {
		int size;
		getData(&size, sizeof(size));
		char* str = new char[size];
		getData(str, size);
		data = str;
		delete str;
		return *this;
	}
};

#endif