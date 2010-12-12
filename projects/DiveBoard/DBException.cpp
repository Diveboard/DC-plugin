#include "DBException.h"


DBException::DBException(std::string t, std::exception e)
{
	text = t;
	ex = e;
}

DBException::DBException(std::string t)
{
	text = t;
}

DBException::DBException()
{
	text = "Unqualified DBO Exception";
}

DBException::~DBException(void)
{
}

const char* DBException::what() const throw()
{
	return ( (text + "\n" + ex.what()).c_str());
}