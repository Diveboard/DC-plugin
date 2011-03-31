#pragma once

#include "stdafx.h"
#include "Computer.h"
#include <string>
#include <map>

class ComputerFactory
{
public:
	ComputerFactory(void);
	~ComputerFactory(void);

	std::map <std::string, std::string>detectConnectedDevice();
	Computer *createComputer(const std::string &type, const std::string &filename);
	std::string detectConnectedDevice(const std::string &computerType);
	std::map <std::string, std::string> allPorts();
protected:
	void listPorts(std::string &);
	bool mapDevice(std::string identifier, std::string &found);
	std::map <std::string, std::vector<std::string>> recognisedPorts;
};

