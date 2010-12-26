#pragma once

#include "Device.h"
#include "DeviceSuunto.h"
#include "Computer.h"


class ComputerSuunto : public Computer
{
private:
	Device *device;
	bool isConnected;
	unsigned char generate_crc(unsigned char *buffer,int len) ;
	void send_command(unsigned char *commbuffer,int len) ;
	void read(int start,char *retbuffer,int len) ;
    void parse_dive(unsigned char *divebuf,int len,ComputerModel model,DiveData &dive);
	int get_dive(char suunto_dive_which,unsigned char *divebuf,int len) ;
	ComputerStatus status;
public:
	ComputerSuunto(std::string filename);
	virtual ~ComputerSuunto(void);
	ComputerModel _get_model();
	int _get_all_dives(std::string &xml);
	ComputerStatus get_status();
};

