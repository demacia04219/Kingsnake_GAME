#pragma once
#include <malloc.h>

template<class T>
class MemoryPool
{
private:
	static T** _instance;
	static int _count;
	static int _instance_length;

public:

	static void* operator new(size_t size) {
		if (_instance_length == 0) {
			_count = _instance_length = 10;
			_instance = (T**)malloc(sizeof(T*) * _instance_length);
			for (int i = 0; i < _instance_length; i++)
				_instance[i] = (T*)malloc(sizeof(T));
		}
		if (_count == 0) {
			_instance = (T**)realloc(_instance, sizeof(T*) * _instance_length*2);
			_count += _instance_length;
			_instance_length += _instance_length;

			T* _array = (T*)malloc(sizeof(T) * _count);
			for (int i = 0; i < _count; i++)
				_instance[i] = &(_array[i]);
		}
		return _instance[--_count];
	}
	static void operator delete(void* p, size_t size) {
		_instance[_count++] = (T*)p;
	}

	static int getPoolLength() { return _instance_length; }
};
template<class T>
T** MemoryPool<T>::_instance = nullptr;
template<class T>
int MemoryPool<T>::_count = 0;
template<class T>
int MemoryPool<T>::_instance_length = 0;