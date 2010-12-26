#pragma once

#include "Device.h"


class DeviceSuuntoEmu : public ComDevice
{
public:
	DeviceSuuntoEmu(std::string filename);
	~DeviceSuuntoEmu(void);
	virtual void open();
	virtual void run();
};

