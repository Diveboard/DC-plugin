#pragma once
#include <exception>
#include <string>

#include <JSAPI.h>


class DBException :
	public std::exception
{
private:
	std::exception ex;
public:
	DBException(void);
	DBException(std::string t, std::exception e);
	DBException(std::string t);
	~DBException(void);
};

