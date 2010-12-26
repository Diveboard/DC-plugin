#pragma once

#include "stdafx.h"
#include <string>
#include <windows.h>

#define SUUNTO_OK 0

#define SUUNTO_ERR_READ -3
#define SUUNTO_ERR_UNCONNECTED -30
#define SUUNTO_ERR_CRC -4

#define SUUNTO_ERR_CREATEFILE -5
#define SUUNTO_ERR_SETCOMMSTATE -6
#define SUUNTO_ERR_SETSIG -7



typedef enum {
	DTR_STATUS_ON=0,
	DTR_STATUS_OFF
}DTRStatus;

typedef enum {
	RTS_STATUS_ON=0,
	RTS_STATUS_OFF
}RTSStatus;



class ComDevice
{
public:
	virtual void close(); 
	virtual void run()=0;
	virtual void open()=0;
	virtual ~ComDevice();
protected:
	HANDLE hCom;
	void set_dtr(DTRStatus) ;
	void set_rts(RTSStatus);
	std::string filename;
};

