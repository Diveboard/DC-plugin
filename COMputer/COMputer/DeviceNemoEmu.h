#pragma once
#include "device.h"

std::wstring s2ws(const std::string& s);

class DeviceNemoEmu :
	public ComDevice
{
public:
	DeviceNemoEmu(std::string s);
	~DeviceNemoEmu(void);
	virtual void run();
	virtual void open();
};

