#ifndef __ENCODING__
#define __ENCODING__

#include <Windows.h>

class Encoding
{
public:
	static void AsciiToUnicode(const char* _source, wchar_t*& _out_result)
	{
		int size = strlen(_source) + 1;
		_out_result = new wchar_t[size];
		MultiByteToWideChar(CP_ACP, 0, _source, -1, _out_result, size);
	}
	static void AsciiToUnicode(const char* _source, wchar_t*& _out_result, int& _out_size)
	{
		_out_size = strlen(_source) + 1;
		_out_result = new wchar_t[_out_size];
		MultiByteToWideChar(CP_ACP, 0, _source, -1, _out_result, _out_size);
		_out_size = (wstrlen(_out_result) + 1) * 2;
	}
	static void UnicodeToAscii(const wchar_t* _source, char*& _out_result)
	{
		int size = (wstrlen(_source)+1)*2;
		_out_result = new char[size];
		WideCharToMultiByte(CP_ACP, 0, _source, -1, _out_result, size, 0, 0);
	}
	static void UnicodeToAscii(const wchar_t* _source, char*& _out_result, int& _out_size)
	{
		_out_size = (wstrlen(_source) + 1) * 2;
		_out_result = new char[_out_size];
		WideCharToMultiByte(CP_ACP, 0, _source, -1, _out_result, _out_size, 0, 0);
		_out_size = strlen(_out_result) + 1;
	}
	static int wstrlen(const wchar_t* _source)
	{
		int size = 0;
		while (*(_source + size) != '\0')
		{
			size++;
		}
		return size;
	}
};

#endif