#ifndef __NETWORK_EXCEPTION_H__
#define __NETWORK_EXCEPTION_H__

#include <string>

enum NETWORK_ERROR {
	FAIL_PACK,
	FAIL_UNPACK,
	FAIL_ACCEPT
};

class NewtorkException
{
	using string = std::string;
public:
	string msg;
	NETWORK_ERROR errorCode;

	NewtorkException(string msg, NETWORK_ERROR errorCode)
	: msg(msg), errorCode(errorCode){}
};

class PacketException : public NewtorkException
{
public:
	PacketException(const char* msg, NETWORK_ERROR code)
		:NewtorkException(msg, code) {}
};
#endif