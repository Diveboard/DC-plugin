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


void Logger::append(int line, char*file, char * level, std::string s)
{
	append(line, file, level, "%s", s.c_str());
}

void Logger::append(int line, char*file, char *level, char *pstrFormat, ...)
{
	char buff[2048];
	std::string str;
	time_t t;
	tm * ptm;
	
	time(&t);
	ptm = gmtime(&t);

	sprintf(buff, "%04d%02d%02d %02d%02d%02d GMT - %-8s - %s @ %d - ", ptm->tm_year+1900, ptm->tm_mon, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, level, file, line);
	str = buff;

	// format and write the data we were given
	va_list args;
	va_start(args, pstrFormat);
	vsprintf(buff, pstrFormat, args);
	va_end(args);

	str += buff;
	
    append(str);
}

std::vector<std::string> Logger::logs;
std::vector<BinaryData> Logger::binData;

void Logger::binary(std::string type, unsigned char *data, unsigned int len)
{
	std::string buff;

	for (unsigned int i=0; i < len; i++)
		buff += str(boost::format("%02X") % ((unsigned int)(data[i])));

	binary(type, buff);
}


void Logger::binary(std::string type, std::string data)
{
	BinaryData b;
	b.type = type;
	b.data = data;
	binData.push_back(b);
	return;
}

std::string Logger::getBinary()
{
	unsigned int i;
	std::string out;
	std::stringstream ssb;
	for (i=0; i<binData.size();i++)
	{
		ssb << binData[i].type << " - " << binData[i].data;
		ssb << std::endl;
	}
	out = ssb.str();
	//std::string out0 = CT2CA(out);
	return(out);}

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

