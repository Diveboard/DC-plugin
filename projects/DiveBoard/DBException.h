#pragma once
#include <exception>
#include <string>

#include <JSAPI.h>


class DBException :
	public FB::script_error
{
private:
	std::string text;
	std::exception ex;
	std::string fulltxt;
public:
	DBException(void);
	DBException(std::string t, std::exception e);
	DBException(std::string t);
	~DBException(void);
	virtual const char* what();
};

