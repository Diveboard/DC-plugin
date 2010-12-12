#pragma once
#include "stdafx.h"
#include <string>

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
	static void binary(std::string type, unsigned char *data, unsigned int len);
	static void binary(std::string type, std::string data);
};


