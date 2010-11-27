#include "Logger.h"
#include <boost/format.hpp>

#include <stdio.h>


using boost::format;


Logger::Logger(void)
{
}


Logger::~Logger(void)
{
}

void Logger::append(std::string line)
{
	logs.push_back(line);
}

void Logger::append(char *pstrFormat, ...)
{
   //CTime timeWrite;
   //timeWrite = CTime::GetCurrentTime();

   // write the time out
   //CString str = timeWrite.Format("%d %b %y %H:%M:%S - ");
   //refFile.Write(str, str.GetLength());

	std::string str;

   // format and write the data we were given
   va_list args;
   va_start(args, pstrFormat);

	char buff[2048];
	vsprintf(buff, pstrFormat, args);
	
	va_end(args);

	str = buff;
	
   append(str);
   return;
}

std::vector<std::string> Logger::logs;


std::string Logger::toString()
{
	unsigned int i;
	std::string out;
	std::stringstream ssb;
	for (i=0; i<logs.size();i++)
	{
		ssb << logs[i];
		ssb << std::endl;
	}
	out = ssb.str();
	//std::string out0 = CT2CA(out);
	return(out);
}

