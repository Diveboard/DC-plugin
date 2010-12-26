#pragma once
#include "stdafx.h"
#include <string>


#ifdef WIN32
#define __THIS_FILE__ ((strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\'): (__FILE__ - 1)) + 1)
#else
#define __THIS_FILE__ ((strrchr(__FILE__, '/') ? strrchr(__FILE__, '/'): (__FILE__ - 1)) + 1)
#endif

#define LOGCRITICAL(...) Logger::append(__LINE__, __THIS_FILE__, "CRITICAL", __VA_ARGS__)
#define LOGERROR(...) Logger::append(__LINE__, __THIS_FILE__, "ERROR", __VA_ARGS__)
#define LOGWARNING(...) Logger::append(__LINE__, __THIS_FILE__, "WARNING", __VA_ARGS__)
#define LOGINFO(...) Logger::append(__LINE__, __THIS_FILE__, "INFO", __VA_ARGS__)
#define LOGDEBUG(str, ...) Logger::append(__LINE__, __THIS_FILE__, "DEBUG", str, __VA_ARGS__)

#define DBthrowError(...) Logger::addnthrow(__LINE__, __THIS_FILE__, "ERROR", __VA_ARGS__)


std::wstring s2ws(const std::string& s);

typedef struct {
	std::string type;
	std::string data;
} BinaryData;

class Logger
{
private:
	Logger(void);
	~Logger(void);
public:
	static std::vector<std::string> logs;
	static std::vector<BinaryData> binData;
	static std::string toString();
	static std::string getBinary();
	static void append(std::string);
	static void append(char *pstrFormat, ...);
	static void append(int line, char*file, char * level, std::string s);
	static void append(int line, char*file, char * level, char *pstrFormat, ...);
	static void addnthrow(int line, char*file, char * level, std::string s);
	static void addnthrow(int line, char*file, char * level, char *pstrFormat, ...);
	static void binary(std::string type, unsigned char *data, unsigned int len);
	static void binary(std::string type, std::string data);
};


