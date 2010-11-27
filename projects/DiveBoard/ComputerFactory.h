#pragma once

#include "stdafx.h"
#include "Computer.h"
#include <string>


class ComputerFactory
{
public:
	ComputerFactory(void);
	~ComputerFactory(void);

	Computer *detectConnectedDevice();
	Computer *createComputer(const std::string &type, const std::string &filename);
protected:
	void listPorts(std::string &);
	Computer *mapDevice(std::string filename, std::string identifier);

};

