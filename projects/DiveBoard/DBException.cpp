#include "DBException.h"
#include <boost/format.hpp>

DBException::DBException(std::string t, std::exception e)
	: exception((t + " --- " + e.what()).c_str())
{
	ex = e;
}

DBException::DBException(std::string t)
	: exception(t.c_str())
{
}

DBException::DBException()
	: exception("Unqualified DBO Exception")
{
}

DBException::~DBException(void)
{
}
