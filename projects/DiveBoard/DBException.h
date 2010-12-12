#pragma once
#include <exception>
#include <string>

class DBException :
	public std::exception
{
private:
	std::string text;
	std::exception ex;
public:
	DBException(void);
	DBException(std::string t, std::exception e);
	DBException(std::string t);
	~DBException(void);
	virtual const char* what() const throw();
};

