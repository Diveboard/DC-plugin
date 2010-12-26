#include "DBException.h"
#include <boost/format.hpp>

DBException::DBException(std::string t, std::exception e)
	: script_error(t)
{
	text = t;
	ex = e;
	fulltxt = text + " --- " + e.what();
}

DBException::DBException(std::string t)
	: script_error(t)
{
	text = t;
	fulltxt = text;
}

DBException::DBException()
	: script_error("Unqualified DBO Exception")
{
	text = "Unqualified DBO Exception";
	fulltxt = text;
}

DBException::~DBException(void)
{
}

const char* DBException::what()
{
	return ( fulltxt.c_str());
}