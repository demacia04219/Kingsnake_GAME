#include "NetworkException.h"
#include "Packet.h"
#include <stdlib.h>
#include "EncryptManager.h"

/*
	Packets
*/
Packets::Packets() {
	size = 0;
}
Packets::Packets(const Packets& p)
{
	size = p.size;
	packets = p.packets;
}
Packets& Packets::operator =(const Packets& p)
{
	size = p.size;
	packets = p.packets;
	return *this;
}
Packets& Packets::operator =(Packet& p)
{
	size = 0;
	packets.clear();

	if (p.getPackingSize() > 0) {
		packets.push_back(&p);
		size += p.getTotalSize();
	}
	return *this;
}
Packets& Packets::operator +=(Packet& p)
{
	if (p.getPackingSize() > 0) {
		packets.push_back(&p);
		size += p.getTotalSize();
	}
	return *this;
}
Packets& Packets::operator +(Packet& p)
{
	if (p.getPackingSize() > 0) {
		packets.push_back(&p);
		size += p.getTotalSize();
	}
	return *this;
}
int Packets::memcpyWithEncrypt(void* canvas) {
	char* pointer = (char*)canvas;
	for (Packet*data : packets) {
		int _size = data->getTotalSize();
		memcpy(pointer, data->getTotalBuffer(), _size);
		EncryptManager::GetInstance()->encoding(pointer + sizeof(int), data->getPackingSize());
		pointer += _size;
	}
	return getTotalSize();
}
int Packets::memcpyWithEncrypt(void* canvas, int start, int size)
{
	char* pointer = (char*)canvas;
	int sentSize = 0;
	int _size;

	for (Packet* p : packets) {
		if (start > 0) {
			if (p->getTotalSize() <= start) {
				start -= p->getTotalSize();
				continue;
			}
			else {
				_size = (p->getTotalSize() - start < size ? p->getTotalSize() - start : size);
				memcpy(pointer, p->getTotalBuffer() + start, _size);
				int max = ((int)sizeof(int) - start > 0 ? (int)sizeof(int) - start : 0);
				EncryptManager::GetInstance()->encoding(pointer + max, _size - max);
				pointer += _size;
				sentSize += _size;
				size -= _size;
				start = 0;
			}
		}
		else {
			_size = (p->getTotalSize() < size ? p->getTotalSize() : size);
			memcpy(pointer, p->getTotalBuffer(), _size);
			EncryptManager::GetInstance()->encoding(pointer + sizeof(int), _size - (int)sizeof(int));
			pointer += _size;
			sentSize += _size;
			size -= _size;
		}

		if (size == 0) break;
	}
	return sentSize;
}
int Packets::getTotalSize() {
	return size;
}
void Packets::init(bool tryToDelete) {
	if (tryToDelete) {
		for (auto p : packets)
			delete p;
	}
	size = 0;
	packets.clear();
}
Packets Packet::operator+(Packet& p)
{
	Packets _new;
	if (getPackingSize() > 0) {
		_new.packets.push_back(this);
		_new.size += getTotalSize();
	}
	if (p.getPackingSize() > 0) {
		_new.packets.push_back(&p);
		_new.size += p.getTotalSize();
	}
	return _new;
}


/*
	Packet
*/

Packet::Packet(int bufferSize)
{
	this->bufferSize = bufferSize;
	buffer = new char[this->bufferSize + 4];
	ptr = buffer + sizeof(int);
	dataSize = (int*)buffer;
	*dataSize = 0;
}
Packet::Packet(const Packet& p)
{
	bufferSize = p.bufferSize;
	buffer = new char[bufferSize + 4];
	memcpy(buffer, p.buffer, p.bufferSize+4);
	dataSize = (int*)buffer;
	ptr = buffer + (p.ptr - p.buffer);
}
Packet& Packet::operator=(const Packet& p)
{
	delete buffer;
	bufferSize = p.bufferSize;
	buffer = new char[bufferSize + 4];
	memcpy(buffer, p.buffer, p.bufferSize + 4);
	dataSize = (int*)buffer;
	ptr = buffer + (p.ptr - p.buffer);
	return *this;
}
Packet::~Packet()
{
	delete buffer;
}

int Packet::getPackingSize()
{
	return *dataSize;
}
int Packet::getTotalSize()
{
	return ((*dataSize) + 4);
}
int Packet::getBufferSize()
{
	return bufferSize;
}
void Packet::setBufferSize(int size)
{
	int ptr_position = ptr - buffer;
	if (ptr_position > size + 4)	ptr_position = size + 4;

	buffer = (char*)realloc(buffer, size + 4);
	ptr = buffer + ptr_position;
	dataSize = (int*)buffer;
	bufferSize = size;
}
void Packet::initPointer()
{
	ptr = buffer + sizeof(int);
}
void Packet::initBuffer() 
{
	ptr = buffer + sizeof(int);
	*dataSize = 0;
}
char* Packet::getPackingBuffer()
{
	return buffer + sizeof(int);
}
char* Packet::getTotalBuffer() {
	return buffer;
}
void Packet::addData(void* data, int size)
{
	if (getPackingSize() + size > getBufferSize())
	{
		int resize = getBufferSize() * 2;
		if (getPackingSize() + size > resize)
			resize = getPackingSize() + size;

		int ptr_position = ptr - buffer;
		buffer = (char*)realloc(buffer, resize + 4);
		ptr = buffer + ptr_position;
		dataSize = (int*)buffer;
		bufferSize = resize;
	}
	memcpy(ptr, data, size);
	ptr += size;
	*dataSize += size;
}
void Packet::getData(void* data, int size)
{
	if (canGetData(size) == false) {
		throw new PacketException(
			"GetData() Error : 패킷이 가지고 있는 데이터보다 더 많은 데이터를 가져오려 함.",
			NETWORK_ERROR::FAIL_UNPACK
		);
	}
	memcpy(data, ptr, size);
	ptr += size;
}
int Packet::wstrlen(const wchar_t* _source)
{
	int size = 0;
	while (*(_source + size) != '\0')
	{
		size++;
	}
	return size;
}
bool Packet::canGetData(int size)
{
	int readSize = ptr - (buffer + sizeof(int));
	if (getPackingSize() - readSize < size) {
		return false;
	}
	return true;
}