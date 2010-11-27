#pragma once
#include "stdafx.h"
#include <string>

class Logger
{
private:
	Logger(void);
	~Logger(void);
public:
	static std::vector<std::string> logs;
	static std::string toString();
	static void append(std::string);
	static void append(LPCTSTR pstrFormat, ...);
};


