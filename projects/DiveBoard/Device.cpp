/*
 *  Device.cpp
 *  FireBreath
 *
 *  Created by Pascal Manchon on 27/11/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "Device.h"

#include "Logger.h"

#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
/* for strptime prototype - see man strptime */
#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif
#include <time.h>

#ifdef __MACH__
#include <sys/ioctl.h>
#include <sys/termios.h>
#include <stdio.h>
#endif


ComDevice::~ComDevice() 
{
	if (hCom)
		close();
}


void ComDevice::set_rts(RTSStatus status) 
{
#ifdef _WIN32
	if(!EscapeCommFunction(hCom, status?CLRRTS:SETRTS))
		throw DBException("Error setting RTS on COM Port");
#elif __MACH__
	
	unsigned int bits=TIOCM_RTS;
	
	if(ioctl(hCom,status==RTS_STATUS_ON?TIOCMBIS:TIOCMBIC,&bits)==-1)
		throw DBException("Error setting RTS on COM Port");
	
#else	
#error Platform not supported
#endif
}

void ComDevice::set_dtr(DTRStatus status) 
{
#ifdef _WIN32
	if(!EscapeCommFunction(hCom, status?CLRDTR:SETDTR))
		throw DBException("Error setting RTS on COM Port");
#elif __MACH__
	
	unsigned int bits;
	
	if(ioctl(hCom,TIOCMGET,&bits)) 
		throw DBException("Error setting RTS on COM Port");
	if(status==DTR_STATUS_ON) bits|=TIOCM_DTR;
	else bits&=TIOCM_DTR;
	if(ioctl(hCom,TIOCMSET,&bits))
		throw DBException("Error setting RTS on COM Port");
	
#else	
#error Platform not supported
#endif
}


void ComDevice::close() 
{
	LOGINFO("Closing handle");
	set_dtr(DTR_STATUS_OFF);
#ifdef _WIN32
	if (!CloseHandle(hCom))
		LOGINFO("Warning - Error closing COM Port");
#elif __MACH__
	::close(hCom);
#else	
#error Platform not supported
	//todo : tcsetattr(fd,TCSANOW,&old_termios);
#endif
	hCom = NULL;
}
